
#include "modules/benet/enet_node.h"
#include "core/engine.h"

void ENetNode::_notification(int p_what) {
	if(!is_inside_tree() || Engine::get_singleton()->is_editor_hint() || !network_peer.is_valid())
		return;

	if(p_what == NOTIFICATION_PHYSICS_PROCESS) {
		if(poll_mode == MODE_PHYSICS)
			_network_poll();
		if(signal_mode == MODE_PHYSICS)
			_network_process();
	} else if (p_what == NOTIFICATION_PROCESS) {
		if(poll_mode == MODE_IDLE)
			_network_poll();
		if(signal_mode == MODE_IDLE)
			_network_process();
	}
}

void ENetNode::_update_process_mode() {

	bool idle = signal_mode == MODE_IDLE || poll_mode == MODE_IDLE;
	bool physics = signal_mode == MODE_PHYSICS || poll_mode == MODE_PHYSICS;

	if (is_physics_processing() && !physics) {
		set_physics_process(false);
	}
	if (is_processing() && !idle) {
		set_process(false);
	}

	if(idle && !is_processing()) {
		set_process(true);
	}
	if(physics && !is_physics_processing()) {
		set_physics_process(true);
	}
}

void ENetNode::set_signal_mode(NetProcessMode p_mode) {

	if(signal_mode == p_mode)
		return;

	signal_mode = p_mode;

	_update_process_mode();
}

ENetNode::NetProcessMode ENetNode::get_signal_mode() const{
	return signal_mode;
}

void ENetNode::set_poll_mode(NetProcessMode p_mode) {

	if(poll_mode == p_mode)
		return;

	poll_mode = p_mode;

	_update_process_mode();
}

ENetNode::NetProcessMode ENetNode::get_poll_mode() const{
	return poll_mode;
}

void ENetNode::set_network_peer(const Ref<ENetPacketPeer>& p_network_peer) {
	if (network_peer.is_valid()) {
		network_peer->disconnect("peer_connected",this,"_network_peer_connected");
		network_peer->disconnect("peer_disconnected",this,"_network_peer_disconnected");
		network_peer->disconnect("connection_succeeded",this,"_connected_to_server");
		network_peer->disconnect("connection_failed",this,"_connection_failed");
		network_peer->disconnect("server_disconnected",this,"_server_disconnected");
		connected_peers.clear();
		//path_get_cache.clear();
		//path_send_cache.clear();
		//last_send_cache_id=1;
	}

	ERR_EXPLAIN("Supplied NetworkedNetworkPeer must be connecting or connected.");
	ERR_FAIL_COND(p_network_peer.is_valid() && p_network_peer->get_connection_status()==NetworkedMultiplayerPeer::CONNECTION_DISCONNECTED);

	network_peer=p_network_peer;

	if (network_peer.is_valid()) {
		network_peer->connect("peer_connected",this,"_network_peer_connected");
		network_peer->connect("peer_disconnected",this,"_network_peer_disconnected");
		network_peer->connect("connection_succeeded",this,"_connected_to_server");
		network_peer->connect("connection_failed",this,"_connection_failed");
		network_peer->connect("server_disconnected",this,"_server_disconnected");
	}
}

bool ENetNode::is_network_server() const {

	ERR_FAIL_COND_V(!network_peer.is_valid(),false);
	return network_peer->is_server();

}

int ENetNode::get_network_unique_id() const {

	ERR_FAIL_COND_V(!network_peer.is_valid(),0);
	return network_peer->get_unique_id();
}

Error ENetNode::kick_client(int p_id) {

	ERR_FAIL_COND_V(!network_peer.is_valid(),ERR_UNCONFIGURED);

	return network_peer->disconnect_peer(p_id);
}

void ENetNode::_network_peer_connected(int p_id) {

	connected_peers.insert(p_id);
	//path_get_cache.insert(p_id,PathGetCache());
	emit_signal("network_peer_connected",p_id);
}

void ENetNode::_network_peer_disconnected(int p_id) {

	connected_peers.erase(p_id);
	//path_get_cache.erase(p_id); //I no longer need your cache, sorry
	emit_signal("network_peer_disconnected",p_id);
}

void ENetNode::_connected_to_server() {

	emit_signal("connected_to_server");
}

void ENetNode::_connection_failed() {

	emit_signal("connection_failed");
}

void ENetNode::_server_disconnected() {

	emit_signal("server_disconnected");
}

Error ENetNode::broadcast(const PoolVector<uint8_t> &p_packet, int p_channel) {
	return put_packet(NetworkedMultiplayerPeer::TRANSFER_MODE_RELIABLE, 0, p_packet, p_channel);
}

Error ENetNode::send(int p_id, const PoolVector<uint8_t> &p_packet, int p_channel) {
	return put_packet(NetworkedMultiplayerPeer::TRANSFER_MODE_RELIABLE, p_id, p_packet, p_channel);
}

Error ENetNode::broadcast_unreliable(const PoolVector<uint8_t> &p_packet, int p_channel) {
	return put_packet(NetworkedMultiplayerPeer::TRANSFER_MODE_UNRELIABLE, 0, p_packet, p_channel);
}

Error ENetNode::send_unreliable(int p_id, const PoolVector<uint8_t> &p_packet, int p_channel) {
	return put_packet(NetworkedMultiplayerPeer::TRANSFER_MODE_UNRELIABLE, p_id, p_packet, p_channel);
}

Error ENetNode::broadcast_ordered(const PoolVector<uint8_t> &p_packet, int p_channel) {
	return put_packet(NetworkedMultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED, 0, p_packet, p_channel);
}

Error ENetNode::send_ordered(int p_id, const PoolVector<uint8_t> &p_packet, int p_channel) {
	return put_packet(NetworkedMultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED, p_id, p_packet, p_channel);
}

Error ENetNode::put_packet(NetworkedMultiplayerPeer::TransferMode p_mode, int p_target, const PoolVector<uint8_t> &p_packet, int p_channel) {
	ERR_FAIL_COND_V(!network_peer.is_valid(),ERR_UNCONFIGURED);

	network_peer->set_transfer_mode(p_mode);
	network_peer->set_target_peer(p_target);
	return network_peer->_put_packet_channel(p_packet, p_channel);
}

void ENetNode::_network_poll() {

	if (!network_peer.is_valid() || network_peer->get_connection_status()==NetworkedMultiplayerPeer::CONNECTION_DISCONNECTED)
		return;

	network_peer->poll();
}

void ENetNode::_network_process() {

	if (!network_peer.is_valid() || network_peer->get_connection_status()==NetworkedMultiplayerPeer::CONNECTION_DISCONNECTED)
		return;

	while(network_peer->get_available_packet_count()) {

		int sender = network_peer->get_packet_peer();
		int channel = network_peer->get_packet_channel();

		if(channel==-1) {

			int len;
			const uint8_t *packet;

			Error err = network_peer->get_packet(&packet,len);
			if (err!=OK) {
				ERR_PRINT("Error getting packet!");
			}

			_network_process_packet(sender,packet,len);

		} else {

			PoolVector<uint8_t> pkt;

			Error err = network_peer->get_packet_buffer(pkt);
			if (err!=OK) {
				ERR_PRINT("Error getting packet!");
			}

			if(sender == 1) {
				emit_signal("server_packet", channel, pkt);
			}
			else {
				emit_signal("peer_packet", sender, channel, pkt);
			}

		}

		if (!network_peer.is_valid()) {
			break; //it's also possible that a packet or RPC caused a disconnection, so also check here
		}
	}


}

void ENetNode::_network_process_packet(int p_from, const uint8_t* p_packet, int p_packet_len) {
	// Not implemented yet!
/*
	ERR_FAIL_COND(p_packet_len<5);

	uint8_t packet_type = p_packet[0];

	switch(packet_type) {

		case NETWORK_COMMAND_REMOTE_CALL:
		case NETWORK_COMMAND_REMOTE_SET: {

			ERR_FAIL_COND(p_packet_len<5);
			uint32_t target = decode_uint32(&p_packet[1]);


			Node *node=NULL;

			if (target&0x80000000) {

				int ofs = target&0x7FFFFFFF;
				ERR_FAIL_COND(ofs>=p_packet_len);

				String paths;
				paths.parse_utf8((const char*)&p_packet[ofs],p_packet_len-ofs);

				NodePath np = paths;

				node = get_root()->get_node(np);
				if (node==NULL) {
					ERR_EXPLAIN("Failed to get path from RPC: "+String(np));
					ERR_FAIL_COND(node==NULL);
				}
			} else {

				int id = target;

				Map<int,PathGetCache>::Element *E=path_get_cache.find(p_from);
				ERR_FAIL_COND(!E);

				Map<int,PathGetCache::NodeInfo>::Element *F=E->get().nodes.find(id);
				ERR_FAIL_COND(!F);

				PathGetCache::NodeInfo *ni = &F->get();
				//do proper caching later

				node = get_root()->get_node(ni->path);
				if (node==NULL) {
					ERR_EXPLAIN("Failed to get cached path from RPC: "+String(ni->path));
					ERR_FAIL_COND(node==NULL);
				}


			}

			ERR_FAIL_COND(p_packet_len<6);

			//detect cstring end
			int len_end=5;
			for(;len_end<p_packet_len;len_end++) {
				if (p_packet[len_end]==0) {
					break;
				}
			}

			ERR_FAIL_COND(len_end>=p_packet_len);

			StringName name = String::utf8((const char*)&p_packet[5]);




			if (packet_type==NETWORK_COMMAND_REMOTE_CALL) {

				if (!node->can_call_rpc(name))
					return;

				int ofs = len_end+1;

				ERR_FAIL_COND(ofs>=p_packet_len);

				int argc = p_packet[ofs];
				Vector<Variant> args;
				Vector<const Variant*> argp;
				args.resize(argc);
				argp.resize(argc);

				ofs++;

				for(int i=0;i<argc;i++) {

					ERR_FAIL_COND(ofs>=p_packet_len);
					int vlen;
					Error err = decode_variant(args[i],&p_packet[ofs],p_packet_len-ofs,&vlen);
					ERR_FAIL_COND(err!=OK);
					//args[i]=p_packet[3+i];
					argp[i]=&args[i];
					ofs+=vlen;
				}

				Variant::CallError ce;

				node->call(name,argp.ptr(),argc,ce);
				if (ce.error!=Variant::CallError::CALL_OK) {
					String error = Variant::get_call_error_text(node,name,argp.ptr(),argc,ce);
					error="RPC - "+error;
					ERR_PRINTS(error);
				}

			} else {

				if (!node->can_call_rset(name))
					return;

				int ofs = len_end+1;

				ERR_FAIL_COND(ofs>=p_packet_len);

				Variant value;
				decode_variant(value,&p_packet[ofs],p_packet_len-ofs);

				bool valid;

				node->set(name,value,&valid);
				if (!valid) {
					String error = "Error setting remote property '"+String(name)+"', not found in object of type "+node->get_type();
					ERR_PRINTS(error);
				}
			}

		} break;
		case NETWORK_COMMAND_SIMPLIFY_PATH: {

			ERR_FAIL_COND(p_packet_len<5);
			int id = decode_uint32(&p_packet[1]);

			String paths;
			paths.parse_utf8((const char*)&p_packet[5],p_packet_len-5);

			NodePath path = paths;

			if (!path_get_cache.has(p_from)) {
				path_get_cache[p_from]=PathGetCache();
			}

			PathGetCache::NodeInfo ni;
			ni.path=path;
			ni.instance=0;

			path_get_cache[p_from].nodes[id]=ni;


			{
				//send ack

				//encode path
				CharString pname = String(path).utf8();
				int len = encode_cstring(pname.get_data(),NULL);

				Vector<uint8_t> packet;

				packet.resize(1+len);
				packet[0]=NETWORK_COMMAND_CONFIRM_PATH;
				encode_cstring(pname.get_data(),&packet[1]);

				network_peer->set_transfer_mode(NetworkedMultiplayerPeer::TRANSFER_MODE_RELIABLE);
				network_peer->set_target_peer(p_from);
				network_peer->put_packet(packet.ptr(),packet.size(),0);
			}
		} break;
		case NETWORK_COMMAND_CONFIRM_PATH: {

			String paths;
			paths.parse_utf8((const char*)&p_packet[1],p_packet_len-1);

			NodePath path = paths;

			PathSentCache *psc = path_send_cache.getptr(path);
			ERR_FAIL_COND(!psc);

			Map<int,bool>::Element *E=psc->confirmed_peers.find(p_from);
			ERR_FAIL_COND(!E);
			E->get()=true;
		} break;
	}
*/
}

void ENetNode::_bind_methods() {
	ADD_SIGNAL( MethodInfo("network_peer_connected",PropertyInfo(Variant::INT,"id")));
	ADD_SIGNAL( MethodInfo("network_peer_disconnected",PropertyInfo(Variant::INT,"id")));

	ADD_SIGNAL( MethodInfo("connected_to_server"));
	ADD_SIGNAL( MethodInfo("connection_failed"));
	ADD_SIGNAL( MethodInfo("server_disconnected"));

	ADD_SIGNAL( MethodInfo("server_packet",PropertyInfo(Variant::INT,"channel"),PropertyInfo(Variant::POOL_BYTE_ARRAY,"packet")));
	ADD_SIGNAL( MethodInfo("peer_packet",PropertyInfo(Variant::INT,"peer"),PropertyInfo(Variant::INT,"channel"),PropertyInfo(Variant::POOL_BYTE_ARRAY,"packet")));

	ClassDB::bind_method(D_METHOD("set_network_peer","peer:ENetPacketPeer"),&ENetNode::set_network_peer);
	ClassDB::bind_method(D_METHOD("_network_peer_connected"),&ENetNode::_network_peer_connected);
	ClassDB::bind_method(D_METHOD("_network_peer_disconnected"),&ENetNode::_network_peer_disconnected);
	ClassDB::bind_method(D_METHOD("_connected_to_server"),&ENetNode::_connected_to_server);
	ClassDB::bind_method(D_METHOD("_connection_failed"),&ENetNode::_connection_failed);
	ClassDB::bind_method(D_METHOD("_server_disconnected"),&ENetNode::_server_disconnected);

	// Basic infos
	ClassDB::bind_method(D_METHOD("is_network_server"),&ENetNode::is_network_server);
	ClassDB::bind_method(D_METHOD("get_network_unique_id"),&ENetNode::get_network_unique_id);


	// Signal Handling
	BIND_CONSTANT(MODE_IDLE);
	BIND_CONSTANT(MODE_PHYSICS);
	ClassDB::bind_method(D_METHOD("set_signal_mode","mode"),&ENetNode::set_signal_mode);
	ClassDB::bind_method(D_METHOD("get_signal_mode"),&ENetNode::get_signal_mode);
	ADD_PROPERTYNZ( PropertyInfo(Variant::INT,"signal_mode",PROPERTY_HINT_ENUM,"Idle,Fixed"),"set_signal_mode","get_signal_mode");
	ClassDB::bind_method(D_METHOD("set_poll_mode","mode"),&ENetNode::set_poll_mode);
	ClassDB::bind_method(D_METHOD("get_poll_mode"),&ENetNode::get_poll_mode);
	ADD_PROPERTYNZ( PropertyInfo(Variant::INT,"poll_mode",PROPERTY_HINT_ENUM,"Idle,Fixed"),"set_poll_mode","get_poll_mode");


	// General purpose method
	ClassDB::bind_method(D_METHOD("put_packet", "mode", "target", "pkt","channel"),&ENetNode::put_packet);
	ClassDB::bind_method(D_METHOD("kick_client", "id"),&ENetNode::kick_client);

	// Reliable
	ClassDB::bind_method(D_METHOD("broadcast", "pkt","channel"),&ENetNode::broadcast);
	ClassDB::bind_method(D_METHOD("send", "target", "pkt","channel"),&ENetNode::send);
	// Unreliable
	ClassDB::bind_method(D_METHOD("broadcast_unreliable", "pkt","channel"),&ENetNode::broadcast_unreliable);
	ClassDB::bind_method(D_METHOD("send_unreliable", "target", "pkt","channel"),&ENetNode::send_unreliable);
	// Ordered
	ClassDB::bind_method(D_METHOD("broadcast_ordered", "pkt","channel"),&ENetNode::broadcast_ordered);
	ClassDB::bind_method(D_METHOD("send_ordered", "target", "pkt","channel"),&ENetNode::send_ordered);
}

ENetNode::ENetNode() {
	poll_mode = MODE_IDLE;
	signal_mode = MODE_IDLE;

	network_peer = Ref<ENetPacketPeer>();

	_update_process_mode();
}

ENetNode::~ENetNode() {

}



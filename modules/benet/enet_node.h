#ifndef NETWORKED_MULTIPLAYER_NODE_H
#define NETWORKED_MULTIPLAYER_NODE_H

#include "scene/main/node.h"
#include "io/marshalls.h"
#include "io/networked_multiplayer_peer.h"
#include "modules/benet/enet_packet_peer.h"

class ENetNode: public Node {

	OBJ_TYPE( ENetNode, Node );

public:

	enum NetProcessMode {
		MODE_IDLE,
		MODE_FIXED
	};

private:

	enum NetworkCommands {
		NETWORK_COMMAND_REMOTE_CALL,
		NETWORK_COMMAND_REMOTE_SET,
		NETWORK_COMMAND_SIMPLIFY_PATH,
		NETWORK_COMMAND_CONFIRM_PATH,
	};

	//path sent caches
	struct PathSentCache {
		Map<int,bool> confirmed_peers;
		int id;
	};

	//path get caches
	struct PathGetCache {
		struct NodeInfo {
			NodePath path;
			ObjectID instance;
		};

		Map<int,NodeInfo> nodes;
	};

	Vector<uint8_t> packet_cache;
	Map<int,PathGetCache> path_get_cache;
	HashMap<NodePath,PathSentCache> path_send_cache;
	int last_send_cache_id;

	Ref<ENetPacketPeer> network_peer;
	Set<int> connected_peers;
	NetProcessMode poll_mode;
	NetProcessMode signal_mode;

	void _network_poll();
	void _network_process();
	void _network_process_packet(int p_from, const uint8_t* p_packet, int p_packet_len);

	void _network_peer_connected(int p_id);
	void _network_peer_disconnected(int p_id);

	void _connected_to_server();
	void _connection_failed();
	void _server_disconnected();
	void _update_process_mode();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void rpcp(int p_peer_id,bool p_unreliable,Node *p_node,const StringName& p_method,const Variant** p_arg,int p_argcount);
	void rsetp(int p_peer_id,bool p_unreliable,Node *p_node, const StringName& p_property,const Variant& p_value);
	void _rpc(Node* p_from,int p_to,bool p_unreliable,bool p_set,const StringName& p_name,const Variant** p_arg,int p_argcount);


	void set_network_peer(const Ref<ENetPacketPeer>& p_network_peer);

	void set_signal_mode(NetProcessMode p_mode);
	NetProcessMode get_signal_mode() const;
	void set_poll_mode(NetProcessMode p_mode);
	NetProcessMode get_poll_mode() const;

	bool is_network_server() const;
	int get_network_unique_id() const;
	Error kick_client(int p_id);

	Error broadcast(const DVector<uint8_t> &p_packet, int p_channel=0);
	Error send(int p_id, const DVector<uint8_t> &p_packet, int p_channel=0);
	Error broadcast_unreliable(const DVector<uint8_t> &p_packet, int p_channel=0);
	Error send_unreliable(int p_id, const DVector<uint8_t> &p_packet, int p_channel=0);
	Error broadcast_ordered(const DVector<uint8_t> &p_packet, int p_channel=0);
	Error send_ordered(int p_id, const DVector<uint8_t> &p_packet, int p_channel=0);
	Error put_packet(NetworkedMultiplayerPeer::TransferMode p_mode, int p_target, const DVector<uint8_t> &p_packet, int p_channel=0);

	ENetNode();
	~ENetNode();

};

VARIANT_ENUM_CAST(ENetNode::NetProcessMode);

#endif

#ifndef NETWORKED_MULTIPLAYER_NODE_H
#define NETWORKED_MULTIPLAYER_NODE_H

#include "scene/main/node.h"
#include "core/io/networked_multiplayer_peer.h"
#include "modules/benet/enet_packet_peer.h"

class ENetNode: public Node {

	GDCLASS( ENetNode, Node );

public:

	enum NetProcessMode {
		MODE_IDLE,
		MODE_PHYSICS
	};

private:

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

	void set_network_peer(const Ref<ENetPacketPeer>& p_network_peer);

	void set_signal_mode(NetProcessMode p_mode);
	NetProcessMode get_signal_mode() const;
	void set_poll_mode(NetProcessMode p_mode);
	NetProcessMode get_poll_mode() const;

	bool is_network_server() const;
	int get_network_unique_id() const;
	Error kick_client(int p_id);

	Error broadcast(const PoolVector<uint8_t> &p_packet, int p_channel=0);
	Error send(int p_id, const PoolVector<uint8_t> &p_packet, int p_channel=0);
	Error broadcast_unreliable(const PoolVector<uint8_t> &p_packet, int p_channel=0);
	Error send_unreliable(int p_id, const PoolVector<uint8_t> &p_packet, int p_channel=0);
	Error broadcast_ordered(const PoolVector<uint8_t> &p_packet, int p_channel=0);
	Error send_ordered(int p_id, const PoolVector<uint8_t> &p_packet, int p_channel=0);
	Error put_packet(NetworkedMultiplayerPeer::TransferMode p_mode, int p_target, const PoolVector<uint8_t> &p_packet, int p_channel=0);

	ENetNode();
	~ENetNode();

};

VARIANT_ENUM_CAST(ENetNode::NetProcessMode);

#endif

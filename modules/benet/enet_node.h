#ifndef NETWORKED_MULTIPLAYER_NODE_H
#define NETWORKED_MULTIPLAYER_NODE_H

#include "scene/main/node.h"
#include "io/networked_multiplayer_peer.h"
#include "modules/benet/enet_packet_peer.h"

class ENetNode: public Node {
	OBJ_TYPE( ENetNode, Node );

	Ref<ENetPacketPeer> network_peer;
	Set<int> connected_peers;

	void _network_poll();
	void _network_process_packet(int p_from, const uint8_t* p_packet, int p_packet_len);

	void _network_peer_connected(int p_id);
	void _network_peer_disconnected(int p_id);

	void _connected_to_server();
	void _connection_failed();
	void _server_disconnected();

protected:
	void _notification(int p_what);
	void _on_idle_frame();
	static void _bind_methods();

public:
	void set_network_peer(const Ref<ENetPacketPeer>& p_network_peer);

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

#endif

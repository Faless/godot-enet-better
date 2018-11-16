
#include "register_types.h"
#include "core/error_macros.h"
#include "enet_packet_peer.h"
#include "enet_node.h"

static bool enet_ok=false;

void register_benet_types() {

	if (enet_initialize() !=0 ) {
		ERR_PRINT("ENet initialization failure");
	} else {
		enet_ok=true;
	}

	ClassDB::register_class<ENetPacketPeer>();
	ClassDB::register_class<ENetNode>();
}

void unregister_benet_types() {

	if (enet_ok)
		enet_deinitialize();

}

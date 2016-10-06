
#include "register_types.h"
#include "error_macros.h"
#include "enet_packet_peer.h"
#include "enet_node.h"

static bool enet_ok=false;

void register_benet_types() {

	if (enet_initialize() !=0 ) {
		ERR_PRINT("ENet initialization failure");
	} else {
		enet_ok=true;
	}

	ObjectTypeDB::register_type<ENetPacketPeer>();
	ObjectTypeDB::register_type<ENetNode>();
}

void unregister_benet_types() {

	if (enet_ok)
		enet_deinitialize();

}

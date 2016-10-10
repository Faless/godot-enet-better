
extends Panel

var client = preload("res://client.xscn")
var clients = []
var server_peer = ENetPacketPeer.new()

var udp_port = 4666

onready var container = get_node("Client/Container")
onready var server_log = get_node("Server/ScrollContainer/ServerLog")
onready var server = get_node("Server/ScrollContainer/ENetNode")
onready var scroll = get_node("Server/ScrollContainer")

func _ready():
	# Sent when a client disconnects
	server.connect("network_peer_disconnected", self, "server_client_disconnect")
	# Sent when a new client connects
	server.connect("network_peer_connected", self, "server_client_connect")
	# Sent when a valid UDP packet is received from a client
	server.connect("peer_packet", self, "on_peer_packet")

	# Create a server on port udp_port with 16 custom channels
	server_peer.create_server(udp_port, 16)
	server.set_network_peer(server_peer)

	set_process(true)


func on_peer_packet(id, cmd, pkt):
	write_log(str(id) + " - CMD: " + str(cmd) + " DATA:" + str(Array(pkt)))

func server_client_connect(id):
	write_log(str(id) + " - Connected")
	clients.append(id)

func server_client_disconnect(id):
	write_log(str(id) + " - Disconnected")



### Scene functions
func _process(delta):
	scroll.set_v_scroll(scroll.get_v_scroll()+1000)

func spawn_client():
	var spawn = client.instance()
	container.add_child(spawn)

func write_log(msg):
	print(msg)
	server_log.set_text(server_log.get_text()+"\n"+str(msg))

func _on_RemoveClient_pressed():
	if clients.size() > 0:
		var c = clients[0]
		server.kick_client(c)
		clients.remove(0)

func _on_AddClient_pressed():
	spawn_client()

func _on_BcastTCP_pressed():
	server.broadcast(RawArray([11,12,23]), 1)

func _on_BcastUDP_pressed():
	server.broadcast_ordered(RawArray([57,12,23,1,1,1,2,3]), 0)


extends Control

var id = 0
var connected = false

var client_peer = ENetPacketPeer.new()
onready var client = get_node("ENetNode")
onready var client_log = get_node("ScrollContainer/ClientLog")
onready var scroll_container = get_node("ScrollContainer")

func _ready():
	# Sent when the client disconnects
	client.connect("server_disconnected", self, "disconnected")
	# Sent when the client disconnects
	client.connect("connection_failed", self, "connect_failed")
	# Sent when the client connects to the server
	client.connect("connected_to_server", self, "connected")
	# Sent when a valid packet is received from the server
	client.connect("server_packet", self, "on_server_packet")
	# Sent when a valid packet is received from another peer
	client.connect("peer_packet", self, "on_peer_packet")

	# Sent when another peer connect to this server (will be called once for every connected client when you first connect)
	client.connect("network_peer_connected", self, "peer_connect")
	# Sent when another peer disconnect from the server
	client.connect("network_peer_disconnected", self, "peer_disconnect")

	# Create a client to 127.0.0.1 on port 4666 with 16 custom channels
	client_peer.create_client("127.0.0.1", 4666, 16)
	client.set_network_peer(client_peer)

	client_log.set_text("")
	write_log("Connecting...")
	set_fixed_process(true)
	set_process(true)

func disconnected():
	write_log("Client Disconnected!")
	call_deferred("queue_free")

func connected():
	write_log("Just Connected! My ID is: " + str(client.get_network_unique_id()) )

func connect_failed():
	write_log("Connection failed!")

func peer_connect(id):
	write_log(str(id) + " - Connect")

func peer_disconnect(id):
	write_log(str(id) + " - Disconnect")

func on_server_packet(cmd, pkt):
	write_log("SERVER - CMD: " + str(cmd) + " DATA:" + str(Array(pkt)))

func on_peer_packet(id, cmd, pkt):
	write_log(str(id), " - CMD: " + str(cmd) + " DATA:" + str(Array(pkt)))

func _process(delta):
	scroll_container.set_v_scroll(scroll_container.get_v_scroll()+100)

### Scene functions
func write_log(msg):
	print(msg)
	client_log.set_text(client_log.get_text()+"\n"+str(msg))

func _on_Button_pressed():
	client.send_ordered(1, RawArray([2,52,12]), 2)

func _on_TCP_pressed():
	client.send(1, RawArray([22,36,89]), 1)

func _exit_tree():
	client_peer.close_connection()

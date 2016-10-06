# About
The `benet` module for godot is a fork of `NetworkedMultiplayerPacketPeer` (although it stays compatible) to allow access to multiple channels and the ability to run multiple clients/servers in the same scene.

The module is composed by two parts:

- `ENetPacketPeer`: The fork of `NetworkedMultiplayerPacketPeer`
  - `create_server`: Add parameter to specify max channels (default 1)
  - `create_client`: Add parameter to specify max channels (default 1)
  - `put_packet_channel`: New method, allow to put a packet in the specified channel

- `ENetNode`: Act like `SceneTree`:
  - Poll on idle time (must be in tree to work!)
  - Emit signals
  - Emit additional `server_packet` and `client_packet` signals when receiving packets
  - Will hopefully support RPC

# Installation
Being a module you will need to recompile godot from source. To do that:

1. Clone the godot engine repository
2. Copy the `benet` folder from this repository into the godot `modules` folder
3. Recompile godot (see http://docs.godotengine.org/en/latest/reference/_compiling.html )

# Usage

For usage examples please refer to the project in the `demo` folder.

The methods should be self explainatory. 

# Disclaimer

> THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

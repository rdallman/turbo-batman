
import SocketServer
import array
import socket
import binascii
import struct
import sys

class UDPHandler(SocketServer.DatagramRequestHandler):

  def handle(self):
    msg = self.request[0].strip()
    sock = self.request[1]

    # TODO check if valid
    cs = sum(map(ord, msg))

    # no, I have not found a better way yet
    msg = binascii.hexlify(msg)
    tml = int(msg[0:4], 16)
    checksum = int(msg[4:6], 16)
    gid = int(msg[6:8], 16)
    rid = int(msg[8:10], 16)
    hosts = binascii.unhexlify(msg[10:])
    hosts = hosts.split('~')[1:]

    print self.client_address[0] + " wrote: "
    print "Length "+str(tml)
    print "Checksum "+str(checksum)
    print "Group: "+str(gid)
    print "Request: "+str(rid)
    print "Hosts to resolve: "
    print "\t\n".join(h for h in hosts)

    # TODO if invalid length
    if 0:
      gid = struct.pack("!b", 127)
      rid = struct.pack("!b", 127)
    else:
      gid = struct.pack("!b", gid)
      rid = struct.pack("!b", rid)

    reply = gid + rid

    # TODO if invalid checksum || length
    if 0:
      reply += struct.pack("!h", 0x0000)
    else:
      for h in hosts:
        try:
          ip = socket.gethostbyname(h)
        except:
          ip = "255.255.255.255"
        reply += socket.inet_aton(ip)

    length = struct.pack("!H", (len(reply) + 3) & 0xFFFF)
    # TODO cs isn't right?
    cs = struct.pack("!B", 0xFF & sum(map(ord, length + reply)))
    reply = length + cs + reply
    sock.sendto(reply, self.client_address)


if __name__ == "__main__":

  HOST, PORT = "", int(sys.argv[1])

  server = SocketServer.UDPServer((HOST, PORT), UDPHandler)

  server.serve_forever()

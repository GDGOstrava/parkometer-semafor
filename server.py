import SocketServer
import time

from conn_conf import *

class ParkOmeterHandler(SocketServer.BaseRequestHandler):
    """
    The request handler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """

    def handle(self):
        # self.request is the TCP socket connected to the client
        print "connection from: {}".format(self.client_address[0])

        # just send back the same data, but upper-cased
        while True:
            data = "volno\n"
            print "sending:", data,
            self.request.sendall(data)
            time.sleep(5)

            data = "obsazeno\n"
            print "sending:", data,
            self.request.sendall(data)
            time.sleep(5)

if __name__ == "__main__":

    # Create the server, binding to localhost on port 9999
    server = SocketServer.TCPServer((HOST, PORT), ParkOmeterHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()

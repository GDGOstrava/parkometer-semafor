import SocketServer
import time
import psycopg2
import psycopg2.extras
import psycopg2.extensions
import select
import socket

from conn_conf import *

class ParkOmeterHandler(SocketServer.BaseRequestHandler):
    """
    The request handler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """
    def server_activate(self):
        self.server.listen(5)

    def handle(self):
        # self.request is the TCP socket connected to the client
        print "connection from: {}".format(self.client_address[0])
        print self.server.db
        cur = self.server.db.cursor()
        cur.execute("LISTEN semafor;")
        # just send back the same data, but upper-cased
        conn= self.server.db
        self.request.settimeout(10)
        i = 0
        while True:
            if select.select([conn],[],[],1) == ([],[],[]):
                if (i % 60 == 0):
                    print 'check db state'
                    cur.execute("SELECT count(*) >= get_size_parking() FROM hub_parking WHERE parked;")
                    full=cur.fetchone()[0]
                    if full:
                        self.server.parking_state = 'obsazeno\n';
                    else:
                        self.server.parking_state = 'volno\n';
                i+=1
                print 'sending last state:', self.server.parking_state,
                self.request.sendall(self.server.parking_state)
            else:
                conn.poll()
                while conn.notifies:
                    notify = conn.notifies.pop(0)
                    print "Got NOTIFY:", notify.pid, notify.channel, notify.payload
                    self.server.parking_state = notify.payload+'\n'
                    self.request.sendall(self.server.parking_state)

    def handle_timeout(self):
        raise Exception('timeout')
           
class ParkOmeterTCPServer(SocketServer.TCPServer):

    def __init__(self, server_address, RequestHandlerClass, bind_and_activate=True):
        SocketServer.TCPServer.__init__(self, server_address, RequestHandlerClass, bind_and_activate)
        self.db=psycopg2.extras.DictConnection(DB_STRING)
        self.db.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_AUTOCOMMIT)
        self.parking_state = None

if __name__ == "__main__":

    # Create the server, binding to localhost on port 9999
    while True:
        try:
            server = ParkOmeterTCPServer((HOST, PORT), ParkOmeterHandler)
            server.allow_reuse_address = True
            server.timeout = 10
            # Activate the server; this will keep running until you
            # interrupt the program with Ctrl-C
            while True:
                server.handle_request()
            #server.serve_forever()

        except socket.error, e:
            print str(e)
            time.sleep(1)

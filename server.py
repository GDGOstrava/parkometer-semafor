import SocketServer
import time
import psycopg2
import psycopg2.extras
import psycopg2.extensions
import select

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
        print self.server.db
        cur = self.server.db.cursor()
        cur.execute("LISTEN semafor;")
        # just send back the same data, but upper-cased
        conn= self.server.db
        while True:
            if select.select([conn],[],[],1) == ([],[],[]):
                print "Timeout"
            else:
                conn.poll()
                while conn.notifies:
                    notify = conn.notifies.pop(0)
                    print "Got NOTIFY:", notify.pid, notify.channel, notify.payload
                    self.request.sendall(notify.payload+'\n')

            
class ParkOmeterTCPServer(SocketServer.TCPServer):

    def __init__(self, server_address, RequestHandlerClass, bind_and_activate=True):
        SocketServer.TCPServer.__init__(self, server_address, RequestHandlerClass, bind_and_activate)
        self.db=psycopg2.extras.DictConnection(DB_STRING)
        self.db.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_AUTOCOMMIT)

if __name__ == "__main__":

    # Create the server, binding to localhost on port 9999
    server = ParkOmeterTCPServer((HOST, PORT), ParkOmeterHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()
    print 'test'

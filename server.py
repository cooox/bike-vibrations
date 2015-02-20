if __name__ == "__main__":
    import http.server
    import socketserver
    import sys

    Handler = http.server.SimpleHTTPRequestHandler
    httpd = socketserver.TCPServer(("localhost", 8000), Handler)

    print("Go to http://localhost:8000/web/ to view the map")

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        httpd.server_close()
        sys.exit(0)

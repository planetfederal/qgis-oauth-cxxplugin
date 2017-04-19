# -*- coding: utf-8 -*-
"""
OAuth 2 Resource Owner Grant Flow Test Server

This simple test server implements the following endpoints:

    /token  : (post) return an access token
    /result : (get) return a short text sentence if token is valid
    / : (get) no token required, return a short text sentence

Environment variables and default values:

    OAUTH2_SERVER_DEFAULT_PORT (8081)
    OAUTH2_SERVER_DEFAULT_SERVERNAME (127.0.0.1)
    OAUTH2_SERVER_USERNAME ("username")
    OAUTH2_SERVER_PASSWORD ("password")

Optional for https:

    OAUTH2_SERVER_AUTHORITY (no default)
    OAUTH2_SERVER_CERTIFICATE (no default)
    OAUTH2_SERVER_KEY (no default)

Sample run with HTTPS enabled:

OAUTH2_SERVER_PORT=47547 OAUTH2_SERVER_HOST=localhost \
    OAUTH2_SERVER_AUTHORITY=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/chains_subissuer-issuer-root_issuer2-root2.pem \
    OAUTH2_SERVER_CERTIFICATE=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/localhost_ssl_cert.pem \
    OAUTH2_SERVER_KEY=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/localhost_ssl_key.pem
    python main.py

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from __future__ import print_function
from future import standard_library
standard_library.install_aliases()

__author__ = 'Alessandro Pasotti'
__date__ = '19/04/2017'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


import os
import sys
import ssl
import signal
from http.server import BaseHTTPRequestHandler, HTTPServer

from oauthlib.oauth2 import RequestValidator, LegacyApplicationServer  # Resource owner


OAUTH2_SERVER_PORT = int(os.environ.get('OAUTH2_SERVER_PORT', '8081'))
OAUTH2_SERVER_HOST = os.environ.get('OAUTH2_SERVER_HOST', '127.0.0.1')
OAUTH2_SERVER_USERNAME = os.environ.get('OAUTH2_SERVER_USERNAME', 'username')
OAUTH2_SERVER_PASSWORD = os.environ.get('OAUTH2_SERVER_PASSWORD', 'password')

OAUTH2_SERVER_AUTHORITY = os.environ.get('OAUTH2_SERVER_AUTHORITY', None)
OAUTH2_SERVER_CERTIFICATE = os.environ.get('OAUTH2_SERVER_CERTIFICATE', None)
OAUTH2_SERVER_KEY = os.environ.get('OAUTH2_SERVER_KEY', None)

HTTPS = (OAUTH2_SERVER_CERTIFICATE is not None and
         os.path.isfile(OAUTH2_SERVER_CERTIFICATE) and
         OAUTH2_SERVER_AUTHORITY is not None and
         os.path.isfile(OAUTH2_SERVER_AUTHORITY) and
         OAUTH2_SERVER_KEY is not None and
         os.path.isfile(OAUTH2_SERVER_KEY))


# Naive token storage implementation
_tokens = {}


class TrueValidator(RequestValidator):
    """Validate username and password"""

    def validate_client_id(self, client_id, request):
        return True

    def authenticate_client(self, request, *args, **kwargs):
        """Wide open"""
        request.client = type("Client", (), {'client_id': 'my_id'})
        return True

    def validate_user(self, username, password, client, request, *args, **kwargs):
        if username == OAUTH2_SERVER_USERNAME and password == OAUTH2_SERVER_PASSWORD:
            return True
        return False

    def validate_grant_type(self, client_id, grant_type, client, request, *args, **kwargs):
        # Clients should only be allowed to use one type of grant.
        return grant_type == 'password'

    def get_default_scopes(self, client_id, request, *args, **kwargs):
        # Scopes a client will authorize for if none are supplied in the
        # authorization request.
        return ('my_scope', )

    def validate_scopes(self, client_id, scopes, client, request, *args, **kwargs):
        """Wide open"""
        return True

    def save_bearer_token(self, token, request, *args, **kwargs):
        # Remember to associate it with request.scopes, request.user and
        # request.client. The two former will be set when you validate
        # the authorization code. Don't forget to save both the
        # access_token and the refresh_token and set expiration for the
        # access_token to now + expires_in seconds.
        _tokens[token['access_token']] = token

    def validate_bearer_token(self, token, scopes, request):
        """Check the token"""
        return token in _tokens


validator = TrueValidator()
oauth_server = LegacyApplicationServer(validator)


class Handler(BaseHTTPRequestHandler):

    def __init__(self, request, client_address, server):
        BaseHTTPRequestHandler.__init__(self, request, client_address, server)

    def __handle_response(self, code, headers, payload):
        self.send_response(code)
        for k, v in headers.items():
            self.send_header(k, v)
        self.end_headers()
        self.wfile.write(payload)

    def do_GET(self):
        """Check oauth valid token"""
        uri = self.path
        http_method = 'get'
        headers = self.headers.dict
        if self.path == '/result':
            result, request = oauth_server.verify_request(uri, http_method, '', headers)
            if result:
                code = 200
                payload = "Your token is valid! Here is your pearl of wisdom: 'I can't change the direction of the wind, but I can adjust my sails to always reach my destination.' (Jimmy Dean)"
            else:
                code = 403
                payload = "Forbidden (invalid token)!"
        elif self.path == '/':
            code = 200
            payload = "This is the test OAuth server, no auth required for this page and here is your pearl of wisdom: 'It's not what you look at that matters, it's what you see.' (Henry David Thoreau)"
        else:
            code = 404
            payload = 'Not found'
        self.__handle_response(code, {}, payload)

    def do_POST(self):
        content_len = int(self.headers.get('content-length', 0))
        body = self.rfile.read(content_len).decode()
        uri = self.path
        http_method = 'post'
        headers = self.headers.dict
        if self.path == '/token':
            headers, payload, code = oauth_server.create_token_response(uri, http_method, body, headers)
        else:
            code = 404
            headers = {}
            payload = 'Not found'
        self.__handle_response(code, headers, payload)


if __name__ == '__main__':

    server = HTTPServer((OAUTH2_SERVER_HOST, OAUTH2_SERVER_PORT), Handler)
    if HTTPS:
        server.socket = ssl.wrap_socket(server.socket,
                                        certfile=OAUTH2_SERVER_CERTIFICATE,
                                        ca_certs=OAUTH2_SERVER_AUTHORITY,
                                        keyfile=OAUTH2_SERVER_KEY,
                                        server_side=True,
                                        ssl_version=ssl.PROTOCOL_TLSv1)
    try:
        print('Starting server on %s://%s:%s, use <Ctrl-C> to stop' %
            ('https' if HTTPS else 'http', OAUTH2_SERVER_HOST, server.server_port), flush=True)
    except TypeError:  # Must be py2
        print('Starting server on %s://%s:%s, use <Ctrl-C> to stop' %
            ('https' if HTTPS else 'http', OAUTH2_SERVER_HOST, server.server_port))

    def signal_handler(signal, frame):
        print("\nExiting test server...")
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)
    server.serve_forever()

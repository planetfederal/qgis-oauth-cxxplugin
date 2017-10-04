#!/bin/bash

# Run the server with HTTPS on port  8443, with QGIS certs


OAUTH2_SERVER_PORT=8443 \
    OAUTH2_SERVER_HOST=localhost \
    OAUTH2_SERVER_AUTHORITY=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/chain_subissuer-issuer-root.pem  \
    OAUTH2_SERVER_CERTIFICATE=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/localhost_ssl_cert.pem \
    OAUTH2_SERVER_KEY=/home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/localhost_ssl_key.pem \
    python main.py

#!/bin/bash

# Get a token from the test endpoint and make a test request

USERNAME="username"
PASSWORD="password"
HOST="https://localhost:8443"

CACERT="--cacert /home/$USER/dev/QGIS/tests/testdata/auth_system/certs_keys/chain_subissuer-issuer-root.pem"


TOKEN=$(curl -s -X POST -d "grant_type=password&username=${USERNAME}&password=${PASSWORD}" ${CACERT} -H "Content-Type: application/x-www-form-urlencoded" ${HOST}/token | python -c "import sys, json; print json.load(sys.stdin)['access_token']")

echo -e "Got token: $TOKEN\n"

# Make valid request
echo "Making a valid request..."
curl -s -X GET ${HOST}/result -H "Authorization: Bearer $TOKEN" ${CACERT}
echo -e "\n"
# Make and invalid request
echo "Making an invalid request..."
curl -s -X GET ${HOST}/result -H "Authorization: Bearer AREEWJEHWJHEWJHEW" ${CACERT}
echo -e "\n"

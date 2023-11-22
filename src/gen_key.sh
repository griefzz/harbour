#!/bin/bash

openssl genpkey -algorithm RSA -out key.pem
openssl req -new -key key.pem -out certificate.csr
openssl x509 -signkey key.pem -in certificate.csr -req -days 365 -out cert.pem
chmod 600 key.key
rm certificate.csr
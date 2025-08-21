#!/usr/bin/env python3
"""
Script CGI lent pour tester les timeouts CGI
"""

import time
import sys
import os

print("Content-Type: text/html\r\n\r\n")
print("<html><head><title>CGI Lent</title></head><body>")
print("<h1>Script CGI qui prend du temps</h1>")

# Simuler un traitement long
while 1:
    print("<p><strong>aaaa!</strong></p>")

print("<p><strong>Traitement terminé!</strong></p>")
print("</body></html>")

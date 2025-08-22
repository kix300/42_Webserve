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
i = 0
while 1:
    i= i + 1

print("<p><strong>Traitement terminé!</strong></p>")
print("</body></html>")

QWebSockets
===========
Introduction
------------
QWebSockets is a pure Qt implementation of WebSockets - both client and server.
It is implemented as a source code module (.pri file), that can easily be embedded into existing Qt projects. It has no other dependencies that Qt.

Features
--------
* Text and binary sockets
* Frame-based and message-based signals
* Works through proxies


Requirements
------------
Qt 5.x

Compliance
----------
QWebSockets is compliant with [RFC6455](http://datatracker.ietf.org/doc/rfc6455/?include_text=1) and has been tested with the [Autobahn Testsuite](http://autobahn.ws/testsuite).
Only tests with **invalid UTF-8 sequences** do **not** pass from the Autobahn Testsuite.

Missing Features
----------------
* WSS protocol
* Checks on valid UTF-8 sequences
* Extensions and sub-protocols

License
-------
This code is licensed under LGPL v3.
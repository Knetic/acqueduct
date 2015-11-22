Acqueduct
==
[![Build Status](https://travis-ci.org/Knetic/acqueduct.svg?branch=master)](https://travis-ci.org/Knetic/acqueduct)

A system to ship raw data from one server to another.

Motivation
==

A huge (and oft-overlooked) portion of managing a service is dealing with its logs. Logs are generated in obscene quantities, at absurd frequencies, for unclear audiences. "Logs" are a catchall way to transport data about a service.

Typically services will reimplement the wheel with services like logback or log4 by creating log appenders, config files, rotation settings, and aggregation policies. Ignoring that all of these things pre-exist in better implementations not directly tied to business logic (in the form of applications like `logrotate`), these logs are almost universally written to disk.

Disk writes should only ever be performed when you have data that _needs_ to be persisted in the location that it's written. Logs, however, spew tons of data onto disk, which is then (typically) periodically copied to some central logshipping server (or a service like Splunk). Writing all this data to disk needlessly wastes iops, slows a service, creates bottlenecks, introduces unnecessary requirements for otherwise small nodes, opens you up to new errors when the disk is full, and leads to complexity when provisioning nodes for a service.

Acqueduct is intended to solve a part of this problem by handling the zip-n-ship portion of the log lifecycle, so that no data ever hits disk.

Operation
==

Acqueduct's client receives data on stdin, and ships it through a connection to an acqueduct server. Generally, clients' stdin is fed through a `mkfifo` named pipe, so that applications can continue to "write" their logs to disk, but those logs will be sent through acqueduct instead.

The Acqueduct server receives the data and sends that data on stdout, in a consistent packet format which identifies the origin server of the data.

Generally, the server will be piped to another application which is capable of forwarding this data to services like Splunk. Acqueduct itself, however, concerns itself only with the effective transport of data between the source and destination.

Other Options
==

Acqueduct is dead simple, and could probably be performed identically by an intricate bash script which handles the named pipe, zipping, netcatting, and so forth. And nobody could really fault you for deciding to do it that way.

But I generally see all portions of a service as being actual code. Not scripts, not run through interpreters, but actual tested-and-in-a-repo code. Duct-taping together an Operations solution may achieve the same physical effect as Acqueduct, but at the cost of stability, maintainability, and effective error handling.

Status
==

This is not ready for production. It's currently being prototyped, and extreme changes are likely to occur before an official 1.0.

License
==

This project is licensed under the MIT general use license. You're free to integrate, fork, and play with this code as you feel fit without consulting the author, as long as you provide proper credit to the author in your works.

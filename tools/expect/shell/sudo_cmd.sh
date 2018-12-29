#!/bin/sh

if { $argc < 3 } {
   puts "invalid argument"
   puts "usage: expect sudo_cmd.sh \[timeout(seconds)] \[password] \[command] \[arg1 arg2 ...]"
   exit 2
}

set times [lindex $argv 0]
set pwd   [lindex $argv 1]
set args  [lrange $argv 2 end]

set timeout $times

spawn sudo su -l -c "$args"

expect {
   "*sudo] password for *" {
      send "$pwd\n"
      interact
   }
   "root's password*" {
      send "$pwd\n"
      interact
   }
   "*assword*" {
      send "$pwd\n"
      interact
   }
   timeout {
      send \x03
      interact
      exit 1
   }
   eof { }
}

exit 0

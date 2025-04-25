#!/bin/bash

SERVER=localhost
PORT=6667

LOG1="client1_output.log"
LOG2="client2_output.log"
LOG3="client3_output.log"
rm -f $LOG1 $LOG2 $LOG3

# Первый клиент: Jo[n\doe~
coproc CLIENT1 { nc $SERVER $PORT | tee "$LOG1"; }
sleep 1
echo "PASS testpass" >&"${CLIENT1[1]}"
sleep 1
echo "NICK Jo[n\\doe~" >&"${CLIENT1[1]}"
sleep 1
echo "USER user1 0 * :Test User 1" >&"${CLIENT1[1]}"
sleep 1
echo "JOIN #chan[one]" >&"${CLIENT1[1]}"
sleep 1
echo "PRIVMSG #chan[one] :Hello from Jo[n\\doe~" >&"${CLIENT1[1]}"
sleep 1

# Второй клиент: Jo{n|doe^
coproc CLIENT2 { nc $SERVER $PORT | tee "$LOG2"; }
sleep 1
echo "PASS testpass" >&"${CLIENT2[1]}"
sleep 1
echo "NICK Jo{n|doe^" >&"${CLIENT2[1]}"
sleep 1
echo "USER user2 0 * :Test User 2" >&"${CLIENT2[1]}"
sleep 1
echo "JOIN #chan{one}" >&"${CLIENT2[1]}"
sleep 1
echo "PRIVMSG #chan{one} :Hello from Jo{n|doe^" >&"${CLIENT2[1]}"
sleep 1

# Третий клиент: DifferentUser
coproc CLIENT3 { nc $SERVER $PORT | tee "$LOG3"; }
sleep 1
echo "PASS testpass" >&"${CLIENT3[1]}"
sleep 1
echo "NICK DifferentUser" >&"${CLIENT3[1]}"
sleep 1
echo "USER user3 0 * :Test User 3" >&"${CLIENT3[1]}"
sleep 1

# PRIVMSG по нику
echo "PRIVMSG Jo{n|doe^ :Hello from Jo[n\\doe~" >&"${CLIENT1[1]}"
sleep 1
echo "PRIVMSG Jo[n\\doe~ :Hi from Jo{n|doe^" >&"${CLIENT2[1]}"
sleep 1

# MULTIPLE PRIVMSG: 2 похожих + 1 разный
echo "PRIVMSG Jo[n\\doe~,Jo{n|doe^,DifferentUser :This is a multi-target message" >&"${CLIENT1[1]}"
sleep 1

# TOPIC
echo "TOPIC #chan[one] :Welcome to the unified channel" >&"${CLIENT1[1]}"
sleep 1

# INVITE and JOIN
echo "INVITE Jo{n|doe^ #chan[one]" >&"${CLIENT1[1]}"
sleep 1
echo "JOIN #chan{one}" >&"${CLIENT2[1]}"
sleep 1

# KICK
echo "KICK #chan[one] Jo{n|doe^ :Testing KICK logic" >&"${CLIENT1[1]}"
sleep 1

# NICK change
echo "NICK jO[n\\doe~" >&"${CLIENT1[1]}"
sleep 1
echo "NICK JO{N|DOE^" >&"${CLIENT2[1]}"
sleep 1

# PING check
echo "PING :still here?" >&"${CLIENT1[1]}"
sleep 1
echo "PING :still here?" >&"${CLIENT2[1]}"
sleep 1
echo "PING :still here?" >&"${CLIENT3[1]}"
sleep 2

# Output result
echo -e "\n===== CLIENT 1 OUTPUT ====="
cat $LOG1
echo -e "\n===== CLIENT 2 OUTPUT ====="
cat $LOG2
echo -e "\n===== CLIENT 3 OUTPUT ====="
cat $LOG3

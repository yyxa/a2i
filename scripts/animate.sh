# #!/bin/bash

# GREEN="\033[0;32m"
# RESET="\033[0m"

# TEXT="a2i"
# WIDTH=30
# DIRECTION=1  # 1 — вправо, -1 — влево
# POS=0

# # Мутации символов
# mutate_char() {
#   local c="$1"
#   local r=$((RANDOM % 10))

#   if (( r < 2 )); then
#     # 20% шанс не менять
#     echo -n "$c"
#   elif (( r < 4 )); then
#     # 20% шанс на верхний регистр
#     echo -n "${c^^}"
#   elif (( r < 6 )); then
#     # 20% шанс заменить на цифру
#     echo -n $((RANDOM % 10))
#   elif (( r < 8 )); then
#     # 20% шанс заменить на спецсимвол
#     SYMBOLS="!@#\$%&*|+~?"
#     echo -n "${SYMBOLS:RANDOM%${#SYMBOLS}:1}"
#   else
#     # 20% шанс заменить на случайную букву
#     LETTERS="abcdefghijklmnopqrstuvwxyz"
#     echo -n "${LETTERS:RANDOM%26:1}"
#   fi
# }

# mutate_text() {
#   local str="$1"
#   local result=""
#   for (( i=0; i<${#str}; i++ )); do
#     result+=$(mutate_char "${str:$i:1}")
#   done
#   echo -n "$result"
# }

# while true; do
#   mutated=$(mutate_text "$TEXT")

#   printf "\r"
#   printf "${GREEN}"
#   printf "%*s" $POS ""
#   printf "%s" "$mutated"
#   printf "${RESET}"
#   sleep 0.05

#   POS=$((POS + DIRECTION))
#   if (( POS >= WIDTH || POS <= 0 )); then
#     DIRECTION=$(( -DIRECTION ))
#   fi
# done

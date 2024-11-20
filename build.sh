#!/bin/bash

g++ -o rocCAD main.cpp -lcurl
if [[ $? -eq 0 ]]; then
  echo -e "\033[32mCompilation Success! ./rocCAD to run\033[0m"
else
  echo -e "\033[31mCompilation failed. Check logs\033[0m"
fi
#!/bin/bash

cd DuckHunt-JS-K8s
docker build --secret id=cached,src=../flag.txt -t ducker-hunt .
cd - 

docker save -o ducker-hunt.tar ducker-hunt

#!/bin/bash

for VAR in {1..3..1}
do
    echo "1"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json

    echo "2"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json

    echo "4"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json

    echo "8"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json

    echo "16"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json

    echo "32"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json

    echo "64"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json

    echo "128"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json

    echo "256"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json

    echo "512"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json

    echo "1024"
    ./bin/register_server register_config.json
    ./bin/register_server register_config.json
    
done
#!/bin/bash
for _ in {1..7}
do
    echo "Log 5000"
    ./bin/register_server register_config.json >> scaling-5k-omega.txt
done

for _ in {1..7}
do
    echo "Lin 5000"
    ./bin/register_server register_config.json >> scaling-5k-omega.txt
done

for _ in {1..7}
do
    echo "Log 10000"
    ./bin/register_server register_config.json >> scaling-10k-omega.txt
done

for _ in {1..7}
do
    echo "Lin 10000"
    ./bin/register_server register_config.json >> scaling-10k-omega.txt
done

for _ in {1..7}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-omega.txt
done

for _ in {1..7}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-omega.txt
done

for _ in {1..7}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-famhe-omega.txt
done

for _ in {1..7}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-famhe-omega.txt
done

for _ in {1..7}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-oblivious-omega.txt
done

for _ in {1..7}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-oblivious-omega.txt
done
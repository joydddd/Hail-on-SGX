#!/bin/bash
for _ in {1..7}
do
    echo "Log 5000"
    ./bin/register_server register_config.json >> scaling-5k-omega-1.txt
done

for _ in {1..7}
do
    echo "Lin 5000"
    ./bin/register_server register_config.json >> scaling-5k-omega-1.txt
done

for _ in {1..7}
do
    echo "Log 10000"
    ./bin/register_server register_config.json >> scaling-10k-omega-1.txt
done

for _ in {1..7}
do
    echo "Lin 10000"
    ./bin/register_server register_config.json >> scaling-10k-omega-1.txt
done

for _ in {1..7}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-omega-1.txt
done

for _ in {1..7}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-omega-1.txt
done

for _ in {1..7}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-famhe-omega-1.txt
done

for _ in {1..7}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-famhe-omega-1.txt
done

for _ in {1..7}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-oblivious-omega-1.txt
done

for _ in {1..7}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-oblivious-omega-1.txt
done
#!/bin/bash
for _ in {1..15}
do
    echo "Log 5000"
    ./bin/register_server register_config.json >> scaling-5k-testing.txt
done

for _ in {1..15}
do
    echo "Lin 5000"
    ./bin/register_server register_config.json >> scaling-5k-testing.txt
done

for _ in {1..15}
do
    echo "Log 10000"
    ./bin/register_server register_config.json >> scaling-10k-testing.txt
done

for _ in {1..15}
do
    echo "Lin 10000"
    ./bin/register_server register_config.json >> scaling-10k-testing.txt
done

for _ in {1..15}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-testing.txt
done

for _ in {1..15}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-testing.txt
done

for _ in {1..15}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-famhe-testing.txt
done

for _ in {1..15}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-famhe-testing.txt
done

for _ in {1..15}
do
    echo "Log 20000"
    ./bin/register_server register_config.json >> scaling-20k-oblivious-testing.txt
done

for _ in {1..15}
do
    echo "Lin 20000"
    ./bin/register_server register_config.json >> scaling-20k-oblivious-testing.txt
done
#!/bin/sh

ninja -C bin-asan && bin-asan/blur

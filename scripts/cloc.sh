#!/usr/bin/env bash
cloc --vcs=git --exclude-list-file=.gitignore --exclude-dir=docs,dep --exclude-ext=md,yml,xml

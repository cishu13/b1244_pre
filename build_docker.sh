#!/bin/bash
NAMESPACE="${1:-codebase_b1244_app}"
docker build -t "$NAMESPACE" .
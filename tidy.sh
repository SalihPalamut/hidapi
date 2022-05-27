#!/bin/bash
astyle --style=allman  --recursive --indent-switches -n --indent=tab --indent-labels --indent-preprocessor -p -U "*.c" "*.h"


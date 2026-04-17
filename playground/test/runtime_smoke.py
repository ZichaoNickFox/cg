#!/usr/bin/env python3

import os
import signal
import subprocess
import sys
import time


def main() -> int:
    if len(sys.argv) < 5:
        print(
            "usage: runtime_smoke.py <window_exe> <working_dir> <cg_runtime> <cg_gl_runtime> <expected-substring> [<expected-substring> ...]",
            file=sys.stderr,
        )
        return 2

    window_exe = sys.argv[1]
    working_dir = sys.argv[2]
    cg_runtime = sys.argv[3]
    cg_gl_runtime = sys.argv[4]
    expected_substrings = sys.argv[5:]

    env = os.environ.copy()
    env["CG_RUNTIME"] = cg_runtime
    env["CG_GL_RUNTIME"] = cg_gl_runtime
    env["CG_WINDOW_WIDTH"] = "640"
    env["CG_WINDOW_HEIGHT"] = "360"

    process = subprocess.Popen(
        [window_exe],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        env=env,
        cwd=working_dir,
    )

    time.sleep(3.0)

    if process.poll() is None:
        process.terminate()
        try:
            output, _ = process.communicate(timeout=5.0)
        except subprocess.TimeoutExpired:
            process.kill()
            output, _ = process.communicate()
    else:
        output, _ = process.communicate()

    if process.returncode not in (0, -signal.SIGTERM, 143):
        print(output, file=sys.stderr, end="")
        print(f"runtime smoke exited unexpectedly with {process.returncode}", file=sys.stderr)
        return 1

    missing = [substring for substring in expected_substrings if substring not in output]
    if missing:
        print(output, file=sys.stderr, end="")
        print(f"missing expected runtime markers: {missing}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

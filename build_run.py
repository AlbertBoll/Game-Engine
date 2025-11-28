import argparse, os, platform, shutil, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent

def run(cmd, cwd=None, env=None):
    print(">>", " ".join(cmd))
    res = subprocess.run(cmd, cwd=cwd, env=env)
    if res.returncode != 0:
        sys.exit(res.returncode)

def cmake_configure(build_dir: Path, config: str, backend: str, fallback: bool, toolchain: Path | None):
    args = [
        "cmake", "-S", str(ROOT), "-B", str(build_dir),
        f"-DCMAKE_BUILD_TYPE={config}",
        f"-DCORE_BACKEND={backend}",
        f"-DENABLE_FETCH_FALLBACK={'ON' if fallback else 'OFF'}",
    ]
    if toolchain:
        args += [f"-DCMAKE_TOOLCHAIN_FILE={toolchain}"]
    run(args)

def cmake_build(build_dir: Path, target = None, jobs: int | None = None):
    args = ["cmake", "--build", str(build_dir)]
    if target:
        args += ["--target", target]
    if jobs:
        args += ["--parallel", str(jobs)]
    run(args)

def guess_exe(build_dir: Path) -> Path:
    # Typical single-config layout (Ninja/Unix Makefiles)
    p = build_dir / "App" / "App"
    if platform.system() == "Windows":
        p = p.with_suffix(".exe")
    return p


def run_exe(exe: Path, build_dir: Path):
    # If Core is SHARED, make sure the loader can find .so/.dylib/.dll
    env = os.environ.copy()
    sysname = platform.system()
    lib_paths = [str(build_dir), str(build_dir / "core")]
    if sysname == "Linux":
        env["LD_LIBRARY_PATH"] = os.pathsep.join(lib_paths + [env.get("LD_LIBRARY_PATH","")])
    elif sysname == "Darwin":
        env["DYLD_LIBRARY_PATH"] = os.pathsep.join(lib_paths + [env.get("DYLD_LIBRARY_PATH","")])
    elif sysname == "Windows":
        env["PATH"] = os.pathsep.join(lib_paths + [env.get("PATH","")])
    run([str(exe)], env=env)


def main():
    ap = argparse.ArgumentParser(description="Build & run CMake App (Debug/Release) with minimal typing.")
    ap.add_argument("mode", choices=["debug", "release", "both", "build-debug", "build-release", "run-debug", "run-release"],
                    help="What to do.")
    ap.add_argument("--backend", choices=["SDL3","GLFW"], default="SDL3", help="Graphics backend selection.")
    ap.add_argument("--fallback", choices=["on","off"], default="on", help="Enable FetchContent fallback if Conan pkgs missing.")
    ap.add_argument("--jobs", "-j", type=int, default=None, help="Parallel build jobs.")
    ap.add_argument("--clean", action="store_true", help="Delete the build dir(s) before building.")
    ap.add_argument("--target", default=None, help="target build.")
    args = ap.parse_args()

    fallback = (args.fallback == "on")

    def build(config: str):
        build_dir = ROOT / "build" / config
        if args.clean and build_dir.exists():
            shutil.rmtree(build_dir)
        build_dir.mkdir(parents=True, exist_ok=True)

        toolchain = None

        cmake_configure(build_dir, "Debug" if config.lower()=="debug" else "Release",
                        args.backend, fallback, toolchain)
        cmake_build(build_dir, args.target, args.jobs)

    def build_run(config: str):
        build_dir = ROOT / "build" / config
        if args.clean and build_dir.exists():
            shutil.rmtree(build_dir)
        build_dir.mkdir(parents=True, exist_ok=True)

        toolchain = None

        cmake_configure(build_dir, "Debug" if config.lower()=="debug" else "Release",
                        args.backend, fallback, toolchain)
        cmake_build(build_dir, args.target, args.jobs)

        exe = guess_exe(build_dir)
        if not exe.exists():
            print(f"ERROR: executable not found at {exe}")
            sys.exit(1)
        run_exe(exe, build_dir)

    if args.mode == "debug":
        build_run("Debug")
    elif args.mode == "release":
        build_run("Release")
    elif args.mode == "both":
        build_run("Debug"); build_run("Release")
    elif args.mode == "build-debug": 
        build("Debug")  
    elif args.mode == "build-release": 
        build("Release")  
    elif args.mode == "run-debug":
        run_exe(guess_exe(ROOT / "build" / "Debug"), ROOT / "build" / "Debug")
    elif args.mode == "run-release":
        run_exe(guess_exe(ROOT / "build" / "Release"), ROOT / "build" / "Release")

if __name__ == "__main__":
    main()
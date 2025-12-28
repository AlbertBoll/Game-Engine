import json
import argparse, os, platform, shutil, subprocess, sys
from pathlib import Path
from typing import Tuple

ROOT = Path(__file__).resolve().parent

def state_path(build_dir: Path) -> Path:
    return build_dir / ".build_run_state.json"

def save_state(build_dir: Path, **kwargs):
    p = state_path(build_dir)
    data = {}
    if p.exists():
        try:
            data = json.loads(p.read_text())
        except Exception:
            data = {}
    data.update({k: v for k, v in kwargs.items() if v is not None})
    p.write_text(json.dumps(data, indent=2))
    print(f"[state] wrote {p}")

def load_state(build_dir: Path) -> dict:
    p = state_path(build_dir)
    if p.exists():
        try:
            return json.loads(p.read_text())
        except Exception:
            return {}
    return {}

def detect_generator(user_gen: str | None):
    if user_gen:
        # print(user_gen)
        #return user_gen[0], None
        return user_gen, None
    if platform.system() == "Windows":
        return "Visual Studio 17 2022", "x64"
    # Linux/macOS
    if shutil.which("ninja"):
        return "Ninja", None
    return "Unix Makefiles", None


def run(cmd, cwd=None, env=None):
    print(">>", " ".join(cmd))
    res = subprocess.run(cmd, cwd=cwd, env=env)
    if res.returncode != 0:
        sys.exit(res.returncode)

def cmake_configure(build_dir: Path, target_engine: str, target_app: str, generator_arch, config: str, backend: str, fallback: bool, enable_imgui: bool, build_shared_libs: bool, toolchain: Path | None):
    # generator, detected_arch = detect_generator(generator_arch)
    generator, detected_arch = generator_arch
    args = [
        "cmake", "-S", str(ROOT), "-B", str(build_dir), "-G", generator,
        f"-DTARGET_ENGINE={target_engine}",
        f"-DTARGET_APP={target_app}",
        f"-DCMAKE_BUILD_TYPE={config}",
        f"-DCORE_WINDOW_BACKEND={backend}",
        f"-DENABLE_FETCH_FALLBACK={'ON' if fallback else 'OFF'}",
        f"-DENABLE_IMGUI={'ON' if enable_imgui else 'OFF'}",
        f"-DBUILD_SHARED_LIBS={'ON' if build_shared_libs else 'OFF'}"
    ]
    if detected_arch and "visual studio" in generator.lower():
        args += ["-A", detected_arch]
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

def guess_exe(build_dir: Path, target_app: str) -> Path:
    # Typical single-config layout (Ninja/Unix Makefiles)
    p = build_dir / "bin" / f"{target_app}"
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
    ap.add_argument("--generator", default=None, help="CMake generator override (e.g. Ninja, Visual Studio 17 2022).")
    ap.add_argument("--mode", choices=["debug", "release", "both", "build-debug", "build-release", "run-debug", "run-release"],
                    help="What to do.")
    ap.add_argument("--window_backend", choices=["SDL3","GLFW"], default="SDL3", help="Graphics backend selection.")
    ap.add_argument("--fallback", choices=["on","off"], default="on", help="Enable FetchContent fallback if Conan pkgs missing.")
    ap.add_argument("--jobs", "-j", type=int, default=None, help="Parallel build jobs.")
    ap.add_argument("--clean", action="store_true", help="Delete the build dir(s) before building.")
    ap.add_argument("--target", default=None, help="target build.")
    ap.add_argument("--enable_imgui", choices=["on", "off"], default="on", help="Enable IMGUI UI or not")
    ap.add_argument("--build_shared_libs", choices=["on", "off"], default="on", help="Build Shared lib or static lib")
    ap.add_argument("--target_engine", default=None, help="name the target engine")
    ap.add_argument("--target_app", default=None, help="name the target app")

    args = ap.parse_args()
    print(args.generator)
    generator_arch = detect_generator(args.generator)
    mode = args.mode
    fallback = (args.fallback == "on")

    enable_imgui = (args.enable_imgui == "on")

    build_shared_libs = (args.build_shared_libs == "on")

    target_engine_name = args.target_engine if args.target_engine else "Engine"
    target_app_name = args.target_app if args.target_app else "App"
 
    window_backend = args.window_backend
    target_to_build = args.target

    jobs = args.jobs


    def build(config: str):
        build_dir = ROOT / "build" / config

        if args.clean and build_dir.exists():
            shutil.rmtree(build_dir)
        build_dir.mkdir(parents=True, exist_ok=True)
        state = load_state(build_dir)
        #if state:
        target_engine_name = state.get("target_engine", "Engine") 
        target_app_name = state.get("target_app", "App") 
        window_backend = state.get("window_backend", "SDL3") 
        jobs = state.get("jobs", 12) 
        enable_imgui = state.get("enable_imgui", True)
        build_shared_libs = state.get("build_shared_libs", True)
           # mode = state.get("mode", mode)


        toolchain = None

        cmake_configure(build_dir, target_engine_name, target_app_name, generator_arch, "Debug" if config.lower()=="debug" else "Release",
                        window_backend, fallback, enable_imgui, build_shared_libs, toolchain)
        cmake_build(build_dir, target_to_build, jobs)
        save_state(build_dir, 
                   mode=mode,
                   target_engine=target_engine_name,
                   target_app=target_app_name,
                   window_backend=window_backend,
                   jobs=jobs,
                   build_shared_libs=build_shared_libs,
                   enable_imgui=enable_imgui)


    def build_run(config: str):
        build_dir = ROOT / "build" / config
        if args.clean and build_dir.exists():
            shutil.rmtree(build_dir)
        build_dir.mkdir(parents=True, exist_ok=True)

        toolchain = None

        cmake_configure(build_dir, target_engine_name, target_app_name, generator_arch, "Debug" if config.lower()=="debug" else "Release",
                        window_backend, fallback, enable_imgui, build_shared_libs, toolchain)
        cmake_build(build_dir, target_to_build, jobs)

        exe = guess_exe(build_dir, target_app_name)
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
        build_dir = ROOT / "build" / "Debug"
        st = load_state(build_dir)
        target_app_name = args.target_app or st.get("target_app")
        run_exe(guess_exe(build_dir, target_app_name), build_dir)
    elif args.mode == "run-release":
        build_dir = ROOT / "build" / "Release"
        st = load_state(build_dir)
        target_app_name = args.target_app or st.get("target_app")
        run_exe(guess_exe(build_dir, target_app_name), build_dir)

if __name__ == "__main__":
    main()
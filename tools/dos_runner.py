"""Execute the pinned DOS toolchain through a selectable host runner.

The runner is deliberately separate from Turbo C/TASM/TLINK identity: it only
executes already-pinned DOS binaries and never supplies compiler inputs.
"""
from dataclasses import dataclass
import hashlib
import os
from pathlib import Path
import shutil
import subprocess


@dataclass(frozen=True)
class DosRunner:
    backend: str
    executable: Path
    # Optional per-program host executables, keyed by the DOS program name
    # (e.g. {'TLINK.EXE': Path(...)}).  Same backend, different emulator build.
    overrides: tuple = ()

    def host_executable(self, program):
        for name, path in self.overrides:
            if name.upper() == Path(program).name.upper():
                return Path(path)
        return self.executable

    def receipt(self):
        digest = lambda path: hashlib.sha256(Path(path).read_bytes()).hexdigest()
        receipt = {'backend': self.backend, 'path': str(self.executable), 'sha256': digest(self.executable)}
        if self.overrides:
            receipt['overrides'] = {name: {'path': str(path), 'sha256': digest(path)} for name, path in self.overrides}
        return receipt

    def _options(self):
        if os.name != 'nt':
            return {}
        startup = subprocess.STARTUPINFO()
        startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startup.wShowWindow = 0
        return {'startupinfo': startup, 'creationflags': subprocess.CREATE_NO_WINDOW}

    def run(self, program, arguments, cwd, environment=None, timeout=600, log_path=None):
        """Run one DOS EXE directly and capture a deterministic host log.

        MS-DOS Player runs the DOS executable directly, avoiding COMMAND.COM
        and an emulator boot. DOSBox remains a reference backend and wraps the
        same command in a temporary mounted batch file.
        """
        cwd = Path(cwd).resolve()
        program = Path(program).resolve()
        env = os.environ.copy()
        if environment:
            env.update({key: str(value) for key, value in environment.items()})
        if self.backend == 'msdos-player':
            tool_dir = str(program.parent)
            # MS-DOS Player has a small DOS environment block. Do not inherit
            # the host's developer environment; Turbo C needs only its tool
            # path and a short writable temporary directory.
            env = {'MSDOS_PATH': tool_dir, 'PATH': tool_dir,
                   'MSDOS_TEMP': str(cwd), 'TEMP': str(cwd), 'TMP': str(cwd)}
            if environment:
                env.update({key: str(value) for key, value in environment.items()})
            command = [str(self.host_executable(program)), '-e', '-v5.00', str(program), *map(str, arguments)]
        elif self.backend == 'dosbox':
            # The program directory is mounted as D: and put on the DOS PATH so that
            # Turbo C finds TASM the same way it does in the batch build; the
            # working directory is C:.  Host paths never reach the DOS side.
            script = cwd / '__RUNNER.BAT'
            command_line = 'D:\\' + program.name + ' ' + ' '.join(map(str, arguments))
            script.write_bytes(('@echo off\r\nset PATH=D:\\\r\n' + command_line + ' > RUNNER.LOG\r\n'
                                'echo %ERRORLEVEL%>RUNNER.RC\r\n').encode('ascii', 'replace'))
            conf = cwd / '__runner.conf'
            conf.write_text('[sdl]\noutput=texture\n[mixer]\nnosound=true\n[autoexec]\n'
                            f'mount c "{cwd}"\nmount d "{program.parent}"\nc:\ncall c:\\__RUNNER.BAT\nexit\n', encoding='utf-8')
            command = [str(self.executable), '-conf', str(conf), '--noprimaryconfig', '-noconsole', '-exit']
        else:
            raise ValueError(f'Unsupported DOS runner backend: {self.backend}')
        result = subprocess.run(command, cwd=cwd, env=env, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, timeout=timeout, **self._options())
        captured = result.stdout
        if self.backend == 'dosbox':
            output = cwd / 'RUNNER.LOG'
            if output.exists():
                captured += output.read_bytes()
            rc = cwd / 'RUNNER.RC'
            if rc.exists():
                try:
                    result.returncode = int(rc.read_text().strip())
                except ValueError:
                    pass
        if log_path:
            Path(log_path).write_bytes(captured)
        return result, command, captured


def _candidate(path):
    return Path(path).expanduser() if path else None


def resolve_runner(lock, backend=None, executable=None):
    """Choose MS-DOS Player when available, otherwise the configured DOSBox."""
    requested = backend or os.environ.get('EMPIRES_DOS_RUNNER') or lock.get('runner', {}).get('default', 'msdos-player')
    override = executable or os.environ.get('MSDOS_PLAYER' if requested == 'msdos-player' else 'DOSBOX')
    if requested == 'msdos-player':
        candidates = [override, lock.get('runner', {}).get('msdos_player_default'), shutil.which('msdos.exe'), shutil.which('msdos')]
        path = next((p for value in candidates if value and (p := _candidate(value)).is_file()), None)
        if path:
            overrides = tuple((name, _candidate(value).resolve()) for name, value in
                              lock.get('runner', {}).get('msdos_player_overrides', {}).items()
                              if _candidate(value) and _candidate(value).is_file())
            return DosRunner('msdos-player', path.resolve(), overrides)
        fallback = lock.get('runner', {}).get('fallback', 'dosbox')
        if backend or fallback != 'dosbox':
            raise ValueError('MS-DOS Player is unavailable; set MSDOS_PLAYER or install msdos.exe')
        requested = 'dosbox'
    if requested == 'dosbox':
        candidates = [override, os.environ.get('DOSBOX'), lock.get('runner', {}).get('dosbox_default'), lock.get('dosbox_default'), shutil.which('dosbox.exe'), shutil.which('dosbox')]
        path = next((p for value in candidates if value and (p := _candidate(value)).is_file()), None)
        if not path:
            raise ValueError('DOSBox fallback is unavailable; set DOSBOX')
        return DosRunner('dosbox', path.resolve())
    raise ValueError(f'Unknown DOS runner backend: {requested}')

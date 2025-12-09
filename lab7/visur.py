import ctypes
import ctypes.wintypes
import sys
import os
from datetime import datetime

class SystemCallMonitor:
    """Учебный мониторинг системных вызовов (Windows API)"""
    
    def __init__(self):
        # Загрузка библиотек Windows API
        self.kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)
        self.advapi32 = ctypes.WinDLL('advapi32', use_last_error=True)
        self.user32 = ctypes.WinDLL('user32', use_last_error=True)
        
        self.setup_structures()
        
    def setup_structures(self):
        """Настройка структур Windows API"""
        # PROCESSENTRY32 структура
        class PROCESSENTRY32(ctypes.Structure):
            _fields_ = [
                ('dwSize', ctypes.c_ulong),
                ('cntUsage', ctypes.c_ulong),
                ('th32ProcessID', ctypes.c_ulong),
                ('th32DefaultHeapID', ctypes.POINTER(ctypes.c_ulong)),
                ('th32ModuleID', ctypes.c_ulong),
                ('cntThreads', ctypes.c_ulong),
                ('th32ParentProcessID', ctypes.c_ulong),
                ('pcPriClassBase', ctypes.c_ulong),
                ('dwFlags', ctypes.c_ulong),
                ('szExeFile', ctypes.c_char * 260)
            ]
        
        self.PROCESSENTRY32 = PROCESSENTRY32
        
        # Установка прототипов функций
        self.setup_function_prototypes()
    
    def setup_function_prototypes(self):
        """Настройка прототипов функций Windows API"""
        
        # CreateToolhelp32Snapshot
        self.kernel32.CreateToolhelp32Snapshot.restype = ctypes.c_void_p
        self.kernel32.CreateToolhelp32Snapshot.argtypes = [
            ctypes.c_ulong, ctypes.c_ulong
        ]
        
        # Process32First
        self.kernel32.Process32First.restype = ctypes.c_bool
        self.kernel32.Process32First.argtypes = [
            ctypes.c_void_p, ctypes.POINTER(self.PROCESSENTRY32)
        ]
        
        # Process32Next
        self.kernel32.Process32Next.restype = ctypes.c_bool
        self.kernel32.Process32Next.argtypes = [
            ctypes.c_void_p, ctypes.POINTER(self.PROCESSENTRY32)
        ]
        
        # OpenProcess
        self.kernel32.OpenProcess.restype = ctypes.c_void_p
        self.kernel32.OpenProcess.argtypes = [
            ctypes.c_ulong, ctypes.c_bool, ctypes.c_ulong
        ]
        
        # CloseHandle
        self.kernel32.CloseHandle.argtypes = [ctypes.c_void_p]
        
        # GetModuleFileName
        self.kernel32.GetModuleFileNameW.restype = ctypes.c_ulong
        self.kernel32.GetModuleFileNameW.argtypes = [
            ctypes.c_void_p, ctypes.c_wchar_p, ctypes.c_ulong
        ]

    def list_processes(self):
        """Получение списка процессов (демонстрация)"""
        print("[SYSCALL] Перечисление процессов...")
        
        TH32CS_SNAPPROCESS = 0x00000002
        hSnapshot = self.kernel32.CreateToolhelp32Snapshot(
            TH32CS_SNAPPROCESS, 0
        )
        
        if hSnapshot:
            process_entry = self.PROCESSENTRY32()
            process_entry.dwSize = ctypes.sizeof(self.PROCESSENTRY32)
            
            if self.kernel32.Process32First(hSnapshot, ctypes.byref(process_entry)):
                count = 0
                while count < 10:  # Ограничиваем вывод для демо
                    pid = process_entry.th32ProcessID
                    exe_file = process_entry.szExeFile.decode('utf-8', errors='ignore')
                    print(f"  PID: {pid}, Executable: {exe_file}")
                    
                    if not self.kernel32.Process32Next(hSnapshot, ctypes.byref(process_entry)):
                        break
                    count += 1
            
            self.kernel32.CloseHandle(hSnapshot)
        
        print(f"[SYSCALL] Показано {min(10, count)} процессов (демо)")

class FileSystemMonitor:
    """Мониторинг файловой системы"""
    
    def __init__(self):
        self.ntdll = ctypes.WinDLL('ntdll', use_last_error=True)
        
    def monitor_file_changes(self, directory="."):
        """Мониторинг изменений файлов (демонстрация)"""
        print("[FILESYSTEM] Мониторинг файловой активности...")
        
        try:
            # Демонстрация отслеживания файлов
            files_before = set(os.listdir(directory))
            
            # Симуляция активности
            test_file = "test_monitor.txt"
            with open(test_file, 'w') as f:
                f.write(f"Тест мониторинга: {datetime.now()}\n")
            
            files_after = set(os.listdir(directory))
            new_files = files_after - files_before
            
            if new_files:
                print(f"[FILESYSTEM] Обнаружены новые файлы: {new_files}")
            
            # Очистка тестового файла
            if os.path.exists(test_file):
                os.remove(test_file)
                
        except Exception as e:
            print(f"[FILESYSTEM] Ошибка мониторинга: {e}")

class NetworkActivitySimulator:
    """Симуляция сетевой активности (без реальных соединений)"""
    
    def simulate_socket_operations(self):
        """Симуляция сетевых операций"""
        print("[NETWORK] Симуляция сетевых операций...")
        
        import socket
        
        # Только создание сокета без реальных соединений
        try:
            # Демонстрация создания сокета
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            print("[NETWORK] Сокет создан (демо)")
            
            # Демонстрация bind (localhost только)
            s.bind(('127.0.0.1', 0))
            print("[NETWORK] Bind выполнен (localhost)")
            
            s.close()
            print("[NETWORK] Сокет закрыт")
            
        except Exception as e:
            print(f"[NETWORK] Демо-ошибка: {e}")

class ProcessInjectionDemo:
    """Демонстрация концепции инъекции кода (без реальной инъекции)"""
    
    def demonstrate_injection_concepts(self):
        """Демонстрация концепций без реальной инъекции"""
        print("[INJECTION] Демонстрация концепций инъекции...")
        
        # Виртуальное выделение памяти (демо)
        MEM_COMMIT = 0x00001000
        PAGE_EXECUTE_READWRITE = 0x40
        
        print(f"[INJECTION] Концепция выделения памяти:")
        print(f"  MEM_COMMIT = 0x{MEM_COMMIT:08X}")
        print(f"  PAGE_EXECUTE_READWRITE = 0x{PAGE_EXECUTE_READWRITE:08X}")
        
        # Демонстрация shellcode (не выполняется)
        shellcode_demo = bytes([
            0x90, 0x90, 0x90,  # NOP sled
            0xC3               # RET
        ])
        
        print(f"[INJECTION] Демо-shellcode размером: {len(shellcode_demo)} байт")
        print("[INJECTION] ВНИМАНИЕ: Реальная инъекция требует прав администратора")

class AntiAnalysisTechniques:
    """Техники анти-анализа (демонстрационные)"""
    
    def check_debugger(self):
        """Проверка наличия отладчика (демо)"""
        print("[ANTI-ANALYSIS] Проверка окружения...")
        
        # Проверка на отладчик через Windows API
        is_debugger_present = ctypes.c_bool()
        self.kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)
        
        if hasattr(self.kernel32, 'IsDebuggerPresent'):
            self.kernel32.IsDebuggerPresent.restype = ctypes.c_bool
            if self.kernel32.IsDebuggerPresent():
                print("[ANTI-ANALYSIS] Обнаружен отладчик (IsDebuggerPresent)")
            else:
                print("[ANTI-ANALYSIS] Отладчик не обнаружен (демо)")
        
        # Проверка времени выполнения (простая демо)
        import time
        start = time.time()
        # Имитация вычислений
        _ = [x*x for x in range(10000)]
        elapsed = time.time() - start
        
        if elapsed > 0.1:  # Простая эвристика
            print(f"[ANTI-ANALYSIS] Подозрительное время выполнения: {elapsed:.4f}с")

def main():
    """Основная функция демонстрации"""
    print("=" * 60)
    print("ЛАБОРАТОРНАЯ РАБОТА: СИСТЕМНЫЕ ВЫЗОВЫ И АНТИ-АНАЛИЗ")
    print("ТОЛЬКО для изолированной виртуальной машины!")
    print("=" * 60)
    
    # Проверка безопасности
    if not check_environment():
        return
    
    try:
        # 1. Мониторинг процессов
        print("\n[ЭТАП 1] Мониторинг системных вызовов")
        monitor = SystemCallMonitor()
        monitor.list_processes()
        
        # 2. Мониторинг файловой системы
        print("\n[ЭТАП 2] Мониторинг файловой системы")
        fs_monitor = FileSystemMonitor()
        fs_monitor.monitor_file_changes()
        
        # 3. Сетевая активность
        print("\n[ЭТАП 3] Сетевая активность (симуляция)")
        net_sim = NetworkActivitySimulator()
        net_sim.simulate_socket_operations()
        
        # 4. Концепции инъекции
        print("\n[ЭТАП 4] Концепции инъекции кода")
        injection_demo = ProcessInjectionDemo()
        injection_demo.demonstrate_injection_concepts()
        
        # 5. Техники анти-анализа
        print("\n[ЭТАП 5] Техники анти-анализа")
        anti_analysis = AntiAnalysisTechniques()
        anti_analysis.check_debugger()
        
        # 6. Создание отчета
        print("\n[ЭТАП 6] Создание отчета")
        generate_report()
        
    except Exception as e:
        print(f"\n[ОШИБКА] {e}")
        print("Возможно, требуется запуск от администратора (для демо)")

def check_environment():
    """Проверка окружения - только VM!"""
    vm_indicators = [
        'vmware', 'virtualbox', 'qemu', 'vbox', 
        'virtual', 'vm', 'kvm', 'xen'
    ]
    
    computer_name = os.environ.get('COMPUTERNAME', '').lower()
    username = os.environ.get('USERNAME', '').lower()
    
    is_vm = False
    for indicator in vm_indicators:
        if indicator in computer_name or indicator in username:
            is_vm = True
            break
    
    if not is_vm:
        print("ПРЕДУПРЕЖДЕНИЕ: Похоже, это не виртуальная машина!")
        print("Этот код предназначен ТОЛЬКО для изолированной VM")
        response = input("Все равно продолжить? (нет/да): ").lower()
        if response != 'да':
            print("Завершение работы...")
            return False
    
    print("Проверка окружения: VM обнаружена (или пользователь подтвердил)")
    return True
    
    print(report)
    
    # Сохранение отчета
    report_file = "lab_report.txt"
    with open(report_file, 'w', encoding='utf-8') as f:
        f.write(report)
    
    print(f"Отчет сохранен в {report_file}")

if __name__ == "__main__":
    main()
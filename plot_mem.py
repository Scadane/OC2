import pandas as pd
import matplotlib.pyplot as plt

def main():
    try:
        df_sys = pd.read_csv("system_memory.csv")
        df_proc = pd.read_csv("process_memory.csv")
    except FileNotFoundError as e:
        print(f"Ошибка: Не найден файл данных: {e.filename}")
        return

    plt.style.use("seaborn-v0_8-whitegrid" if "seaborn-v0_8-whitegrid" in plt.style.available else "default")
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

    # График 1: Системная память
    ax1.plot(df_sys["timestamp"], df_sys["mem_available_mib"], label="MemAvailable", color="#1f77b4", linewidth=2)
    ax1.plot(df_sys["timestamp"], df_sys["mem_free_mib"], label="MemFree", color="#aec7e8", linestyle="--")
    ax1.set_ylabel("Системная память (МиБ)")
    ax1.set_title("Динамика свободной памяти ОС и расхода памяти процессом (до OOM)")
    ax1.legend(loc="upper right")

    # График 2: Память процесса
    ax2.plot(df_proc["timestamp"], df_proc["rss_mib"], label="Process RSS (Физическая)", color="#d62728", linewidth=2)
    ax2.plot(df_proc["timestamp"], df_proc["vmsize_mib"], label="Process VmSize (Виртуальная)", color="#ff9896", linestyle=":")
    ax2.set_xlabel("Время с момента старта (секунды)")
    ax2.set_ylabel("Память процесса (МиБ)")
    ax2.legend(loc="upper left")

    # Отметка точки падения
    last_time = max(df_sys["timestamp"].max(), df_proc["timestamp"].max())
    ax1.axvline(x=last_time, color="black", linestyle="--", alpha=0.7)
    ax2.axvline(x=last_time, color="black", linestyle="--", alpha=0.7)
    ax2.annotate(
        "OOM Killer (SIGKILL)",
        xy=(last_time, df_proc["rss_mib"].iloc[-1]),
        xytext=(last_time * 0.7, df_proc["rss_mib"].iloc[-1] * 0.8),
        arrowprops=dict(facecolor="black", shrink=0.05, width=1, headwidth=6),
        fontweight="bold"
    )

    plt.tight_layout()
    plt.savefig("memory_consumption_plot.png", dpi=300)
    print("[+] График успешно сохранен в 'memory_consumption_plot.png'")
    plt.show()

if __name__ == "__main__":
    main()
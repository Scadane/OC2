import matplotlib.pyplot as plt
import pandas as pd

# Загрузка собранных метрик
data = pd.read_csv("mem_log.csv")

plt.figure(figsize=(10, 5))
plt.plot(
    data["time_sec"],
    data["mem_available_mb"],
    color="crimson",
    linewidth=2,
    label="MemAvailable",
)

plt.title("Динамика свободной памяти при заполнении страниц (mmap)")
plt.xlabel("Время с момента запуска (секунды)")
plt.ylabel("Доступная память (MiB)")
plt.grid(True, linestyle="--", alpha=0.6)
plt.legend()
plt.tight_layout()

plt.savefig("memory_graph.png", dpi=300)
print("График сохранен в memory_graph.png")
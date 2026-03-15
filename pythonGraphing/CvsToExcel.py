import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv("data.csv")

plt.scatter(df["pulseLength"], df["rpm"])
plt.xlabel("Pulse Length")
plt.ylabel("RPM")
plt.title("RPM vs Pulse Length")
plt.grid(True)
plt.show()

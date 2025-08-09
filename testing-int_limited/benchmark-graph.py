import matplotlib.pyplot as plt

class column:
	name: str
	test_cases: list[int]
	bits: list[int]
	times: list[float]
	times_adjusted: list[float]

	def __init__(self, name):
		self.name = name
		self.test_cases = []
		self.bits = []
		self.times = []
		self.times_adjusted = []

columns: list[column]

with open("./benchmark-boost-raw.csv", "r") as data:
	columns = list(map(lambda x: column(x.strip()), data.readline().split(",")))[2:]
	lines = data.readlines()
	for line in lines:
		row = list(map(float, map(lambda x: x.strip(), line.split(","))))
		test_cases = int(row[0])
		bits = int(row[1])
		for i in range(len(columns)):
			columns[i].test_cases.append(test_cases)
			columns[i].bits.append(bits)
			columns[i].times.append(row[i+2])

def divided_plots(columns):
	standard_test_cases = columns[0].test_cases[0]
	bits_cut_offs = [200, 1000, 4000]
	fig, ax = plt.subplots(nrows=len(bits_cut_offs)+1)
	for col in columns:
		cut_off_indices = []
		cut_off_index = 0
		for i in range(len(col.bits)):
			if col.bits[i] > bits_cut_offs[cut_off_index]:
				cut_off_indices.append(i)
				cut_off_index += 1
				if (cut_off_index >= len(bits_cut_offs)): break
		print(cut_off_indices)
		cut_off_indices.append(999)
		for i in range(len(col.times)):
			col.times_adjusted.append(col.times[i]*standard_test_cases/col.test_cases[i])
		start = 0
		end = cut_off_indices[0]
		for i in range(len(bits_cut_offs)):
			ax[i].plot(col.bits[start:end], col.times_adjusted[start:end], "o--")
			start += cut_off_indices[i] - start
			end = cut_off_indices[i+1]
		ax[-1].loglog(col.bits, col.times_adjusted)

	for i in range(len(bits_cut_offs)):
		ax[i].set_xlabel("Bits")
		ax[i].set_ylabel("Time (s)")
	plt.show()

def single_plot(columns):
	fig, ax = plt.subplots()
	fig.set_size_inches(10, 7)
	standard_test_cases = columns[0].test_cases[0]
	for col in columns:
		for i in range(len(col.times)):
			col.times_adjusted.append(col.times[i]*standard_test_cases/col.test_cases[i])
		ax.loglog(col.bits, col.times_adjusted, 'o-')
	ax.legend(list(map(lambda x: x.name, columns)))
	ax.set_xlabel("Bits")
	ax.set_ylabel("Time (s)")
	ax.grid()
	x_labels = [16]
	while x_labels[-1] < 64000:
		x_labels.append(x_labels[-1]*2)

	ax.set_yticks([1, 10, 10**2, 10**3, 10**4, 10**5, 10**6])
	ax.set_yticklabels(map(str, [1, 10, 10**2, 10**3, 10**4, 10**5, 10**6]))
	ax.set_xticks(x_labels)
	ax.set_xticklabels(map(str, x_labels))
	ax.set_title("Boost cpp_int benchmarks")

# divided_plots(columns)
single_plot(columns)
plt.show()
import sys
import matplotlib.pylab as plt

f = open(sys.argv[1], 'r')

while True:
    plot_name = f.readline()
    if plot_name is None or len(plot_name) == 0:
        break

    plot_name = plot_name.rstrip()
    fig, ax = plt.subplots()

    while True:
        label = f.readline().rstrip()
        if label is None or len(label) == 0:
            break
        x = [float(n) for n in f.readline().rstrip().split(' ')]
        y = [float(n) for n in f.readline().rstrip().split(' ')]
        ax.loglog(x, y, label=label.rstrip())

    ax.legend()
    plt.title(plot_name)
    plt.xlabel('Array size')
    plt.ylabel('time (s)')
    plot_name = plot_name.replace(':', '').replace(' ', '_')
    plt.savefig(plot_name + ".png")
    plt.close()

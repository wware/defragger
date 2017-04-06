import random
import gc
from mem import memory_usage, all_info

def waste_memory():
    def random_string():
        s = ""
        for _ in range(10 + int(20 * random.random())):
            s += random.choice("abcdefgh")
        return s
    return [random_string() for _ in range(1000 * 1000)]

# http://stackoverflow.com/questions/23369937

def report_memory():
    # Notice that the GC report indicates that garbage collection is OK.
    print memory_usage(), all_info(gc.get_objects())

def main():
    report_memory()
    x = waste_memory()
    del x
    report_memory()

if __name__ == "__main__":
    main()

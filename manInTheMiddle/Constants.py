
from math import ceil

# Max percentile of noise
NOISE_MARGIN = 0.0005

# Number of bits in a page
PAGE_SIZE = 5#32 * 1024  * 1024 * 8 # 32KB

# Max hamming distance deference allowed because of noise.
NOISE_DISTANCE = ceil(NOISE_MARGIN * PAGE_SIZE)
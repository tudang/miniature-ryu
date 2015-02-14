#!/usr/bin/python

def hamming_distance(s1, s2):
    """Return the Hamming distance between (un)equal-length sequences"""
    #if len(s1) != len(s2):
    #    raise ValueError("Undefined for sequences of unequal length")
    sum = 0
    lines = []
    line = 1
    for ch1, ch2 in zip(s1, s2):
      if ch1 != ch2:
        lines.append(line)
        sum += 1
      line += 1
    return sum
    #return sum(ch1 != ch2 for ch1, ch2 in zip(s1, s2))

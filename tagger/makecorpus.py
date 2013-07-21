from nltk.corpus import brown
from collections import Counter
import itertools

sentences = brown.tagged_sents(simplify_tags=False)
with open('testcorpus.tsv', 'w') as outf:
    for sentence in sentences[1:5000]:
        for word, tag in sentence:
            print >>outf, '%s\t%s' % (tag, word)
        print >>outf, ''
with open('traincorpus.tsv', 'w') as outf:
    for sentence in sentences[5000:]:
        for word, tag in sentence:
            print >>outf, '%s\t%s' % (tag, word)
        print >>outf, ''

sentences = brown.tagged_sents(simplify_tags=True)
with open('simplified_testcorpus.tsv', 'w') as outf:
    for sentence in sentences[1:5000]:
        for word, tag in sentence:
            print >>outf, '%s\t%s' % (tag, word)
        print >>outf, ''

with open('simplified_traincorpus.tsv', 'w') as outf:
    for sentence in sentences[5000:]:
        for word, tag in sentence:
            print >>outf, '%s\t%s' % (tag, word)
        print >>outf, ''

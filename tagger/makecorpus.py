from nltk.corpus import brown
from collections import Counter
import itertools

with open('/Users/josephmalicki/src/tagger/corpus.tsv', 'w') as outf:
    for sentence in brown.tagged_sents(simplify_tags=False):
        for word, tag in sentence:
            print >>outf, '%s\t%s' % (tag, word)
        print >>outf, ''

with open('/Users/josephmalicki/src/tagger/simplified_corpus.tsv', 'w') as outf:
    for sentence in brown.tagged_sents(simplify_tags=True):
        for word, tag in sentence:
            print >>outf, '%s\t%s' % (tag, word)
        print >>outf, ''

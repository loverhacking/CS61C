import sys
import re

from pyspark import SparkContext,SparkConf

def flatMapFunc(document):
    """
    document[0] is the document ID (distinct for each document)
    document[1] is a string of all text in that document

    You will need to modify this code.
    """
    documentID = document[0]
    words = re.findall(r"\w+", document[1])
    return [((word, documentID), idx) for idx, word in enumerate(words)]


def mapFunc(arg):
    """
    You may need to modify this code.
    """
    return (arg, 1)

def reduceFunc(arg1, arg2):
    """
    You may need to modify this code.
    """
    return arg1+arg2

def createIndices(file_name, output="spark-wc-out-createIndices"):
    sc = SparkContext("local[8]", "CreateIndices", conf=SparkConf().set("spark.hadoop.validateOutputSpecs", "false"))
    file = sc.sequenceFile(file_name)

    indices = file.flatMap(flatMapFunc) \
                 .groupByKey() \
                 .mapValues(sorted) \
                 .map(lambda x: (x[0][0], x[0][1], x[1])) \
                 .sortBy(lambda x: (x[0], x[1])) \
                 .map(lambda x: f"({x[0]}  {x[1]}, {' '.join(str(i) for i in x[2])})")

    indices.coalesce(1).saveAsTextFile(output)

""" Do not worry about this """
if __name__ == "__main__":
    argv = sys.argv
    if len(argv) == 2:
        createIndices(argv[1])
    else:
        createIndices(argv[1], argv[2])

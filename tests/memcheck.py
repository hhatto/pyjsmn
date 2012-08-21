import pyjsmn
import time

jsonstring = """\
{
    "name" : "Jack",
    "age" : [ 1, 20, null, true, false, -500.1 ]
}"""
while True:
    print pyjsmn.loads(jsonstring)
    time.sleep(0.1)

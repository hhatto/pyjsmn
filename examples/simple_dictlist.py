import pyjsmn

jsonstr = """\
{
    "name" : "Jack",
    "age" : [ 1, 20, null]
}"""
print pyjsmn.loads(jsonstr)

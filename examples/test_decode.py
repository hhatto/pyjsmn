import pyjsmn

jsonstr = """\
{
    server: "example.com",
    post: 80,
    message: "hello world"
}"""
print pyjsmn.loads(jsonstr)

jsonstr = '{ "name" : "Jack", "age" : 27 }'
print pyjsmn.loads(jsonstr)

jsonstr = """\
{
    "name" : "Jack",
    "age" : [ 1, 20, null, true, false, -500.1 ]
}"""
print pyjsmn.loads(jsonstr)

jsonstr = """["Jack", "age", {"test": 12.1}]"""
print pyjsmn.loads(jsonstr)

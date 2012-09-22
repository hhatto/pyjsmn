import pyjsmn

jsonstr = """\
{
    "name" : "Jack",
    "pc" : {"windows": "XP", "linux": null},
    "mail" : {"title": "good morning",
              "file": ["t.txt", "u.txt"]}
}"""
print pyjsmn.loads(jsonstr)

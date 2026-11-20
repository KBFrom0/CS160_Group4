import sys, re, html, base64, os
def inline(s, base):
    s = html.escape(s, quote=False)
    def img(m):
        p = os.path.join(base, m.group(2))
        data = base64.b64encode(open(p, 'rb').read()).decode()
        return f'<figure><img alt="{m.group(1)}" src="data:image/png;base64,{data}"></figure>'
    s = re.sub(r'!\[([^\]]*)\]\(([^)]+)\)', img, s)
    s = re.sub(r'`([^`]+)`', r'<code>\1</code>', s)
    s = re.sub(r'\*\*([^*]+)\*\*', r'<strong>\1</strong>', s)
    s = re.sub(r'(?<![\w*])_([^_]+)_(?!\w)', r'<em>\1</em>', s)
    return s
def convert(src):
    base = os.path.dirname(src)
    lines = open(src, encoding='utf-8').read().splitlines()
    out, i, title = [], 0, ''
    while i < len(lines):
        l = lines[i]
        if l.startswith('|'):
            rows = []
            while i < len(lines) and lines[i].startswith('|'):
                rows.append([c.strip() for c in lines[i].strip().strip('|').split('|')]); i += 1
            out.append('<table><thead><tr>' + ''.join(f'<th>{inline(c, base)}</th>' for c in rows[0]) + '</tr></thead><tbody>')
            for r in rows[2:]:
                out.append('<tr>' + ''.join(f'<td>{inline(c, base)}</td>' for c in r) + '</tr>')
            out.append('</tbody></table>'); continue
        if l.startswith('- '):
            out.append('<ul>')
            while i < len(lines) and lines[i].startswith('- '):
                out.append(f'<li>{inline(lines[i][2:], base)}</li>'); i += 1
            out.append('</ul>'); continue
        m = re.match(r'(#+) (.*)', l)
        if m:
            n = len(m.group(1)); title = title or m.group(2)
            out.append(f'<h{n}>{inline(m.group(2), base)}</h{n}>')
        elif l.startswith('> '):
            out.append(f'<blockquote>{inline(l[2:], base)}</blockquote>')
        elif l.strip():
            para = [l]
            while i + 1 < len(lines) and lines[i+1].strip() and not re.match(r'[#|>-]', lines[i+1]):
                i += 1; para.append(lines[i])
            out.append('<p>' + '<br>'.join(inline(p, base) for p in para) + '</p>')
        i += 1
    css = """body{font-family:'Segoe UI',Arial,sans-serif;max-width:1000px;margin:40px auto;padding:0 24px;color:#1f2937;line-height:1.55}
h1{border-bottom:3px solid #2563eb;padding-bottom:8px}h2{margin-top:32px;color:#1e3a8a}h3{color:#374151}
table{border-collapse:collapse;width:100%;margin:12px 0;font-size:14px}th,td{border:1px solid #d1d5db;padding:6px 10px;text-align:left;vertical-align:top}
th{background:#eff6ff}tr:nth-child(even) td{background:#f9fafb}code{background:#f3f4f6;padding:1px 5px;border-radius:4px;font-size:90%}
blockquote{border-left:4px solid #93c5fd;margin:12px 0;padding:6px 14px;background:#f8fafc;color:#475569}
figure{margin:12px 0}img{max-width:100%;border:1px solid #d1d5db;border-radius:8px;box-shadow:0 2px 8px rgba(0,0,0,.08)}"""
    doc = f'<!DOCTYPE html>\n<html lang="en"><head><meta charset="utf-8"><title>{html.escape(title)}</title><style>{css}</style></head><body>\n' + '\n'.join(out) + '\n</body></html>\n'
    open(src[:-3] + '.html', 'w', encoding='utf-8').write(doc)
for f in sys.argv[1:]: convert(f); print('ok', f)

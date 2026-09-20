"""Construct the canonical TLINK object order from source module evidence.

The plan is linker input: it never assigns load addresses, changes object
contents, or edits the linked executable.  DATA ranges are compatible
reconstructed source modules until their original translation units can be
proved.
"""


FORMAT = 'empires-canonical-data-link-plan-v1'


def order_data_modules(objects, data_objects, code_objects, plan):
    """Return a response-file object order with source DATA modules placed.

    ``data_objects`` is canonical initialized-DATA order.  Each module in the
    plan owns the next advancing prefix through ``through``; placement after a
    code contribution is the only layout rule.  TLINK still derives every
    segment base and relocation.
    """
    if plan.get('format') != FORMAT:
        raise ValueError('Unsupported canonical DATA link plan')
    modules = plan.get('data_modules')
    if not isinstance(modules, list) or not modules:
        raise ValueError('Canonical DATA link plan has no modules')
    result = list(objects)
    consumed = 0
    placements = []
    for module in modules:
        for key in ('id', 'through', 'after_code', 'before_code'):
            if not isinstance(module.get(key), str):
                raise ValueError('Canonical DATA module lacks identity or placement')
        try:
            stop = next(i for i, (owner, _) in enumerate(data_objects)
                        if owner == module['through']) + 1
        except StopIteration as error:
            raise ValueError('Canonical DATA module boundary is not source DATA') from error
        if stop <= consumed:
            raise ValueError('Canonical DATA modules must advance in source order')
        moving = [name for _, name in data_objects[consumed:stop]]
        try:
            lower = code_objects[module['after_code']]
            upper = code_objects[module['before_code']]
        except KeyError as error:
            raise ValueError('Canonical DATA module code anchor is unavailable') from error
        if lower not in result or upper not in result or result.index(lower) >= result.index(upper):
            raise ValueError('Invalid canonical DATA insertion interval')
        if any(result.count(name) != 1 for name in moving):
            raise ValueError('Canonical DATA object missing or duplicated')
        result = [name for name in result if name not in moving]
        at = result.index(lower) + 1
        result[at:at] = moving
        placements.append({
            'module': module['id'], 'after_code': module['after_code'],
            'before_code': module['before_code'], 'first_component': data_objects[consumed][0],
            'last_component': data_objects[stop - 1][0], 'components': stop - consumed,
        })
        consumed = stop
    data_names = [name for _, name in data_objects]
    if [name for name in result if name in set(data_names)] != data_names:
        raise ValueError('Canonical DATA plan changed DATA contribution order')
    return result, placements

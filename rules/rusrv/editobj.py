def mergeObjs(o_obj, i_obj):
    if i_obj is None or o_obj is None:
        return

    for key in i_obj:
        if key in o_obj and type(i_obj[key]) is dict and type(o_obj[key]) is dict:
            mergeObjs(o_obj[key], i_obj[key])
            continue

        o_obj[key] = i_obj[key]


def pushArray(o_obj, i_edit, i_depth = 0):
    if i_edit is None or o_obj is None:
        return False

    if not isinstance(o_obj, dict):
        return False

    id = 'id'
    if 'keyname' in i_edit:
        id = i_edit['keyname']

    if (not id in i_edit) or (id in o_obj and o_obj[id] == i_edit[id]):
        if i_edit['pusharray'] in o_obj and isinstance(o_obj[i_edit['pusharray']], list):
            pusharray = o_obj[i_edit['pusharray']]
            # Default index for new item - push at the end
            position = len(pusharray)

            # Delete any other items with the same ids as input objects:
            for eobj in i_edit['objects']:
                if id in eobj:
                    for i in range(len(pusharray)):
                        if id in pusharray[i]:
                            if pusharray[i][id] == eobj[id]:
                                del pusharray[i]
                                break

            # Search for an index to insert before:
            if 'id_before' in i_edit:
                for i in range(len(pusharray)):
                    if id in pusharray[i]:
                        if pusharray[i][id] == i_edit['id_before']:
                            position = i

            # Insert objects at given postition:
            for obj in i_edit['objects']:
                pusharray.insert(position, obj)
                position += 1

            return True

    # Search for an array deeper:
    for key in o_obj:
        if isinstance(o_obj[key], list):
            for obj in o_obj[key]:
                if isinstance(obj, dict):
                    if pushArray(obj, i_edit, i_depth + 1):
                        return True
        if isinstance(o_obj[key], dict):
            if pushArray(o_obj[key], i_edit, i_depth + 1):
                return True

    # Create an array in the object root, if it was not found:
    if i_depth == 0:
        o_obj[i_edit['pusharray']] = []
        return pushArray(o_obj, i_edit, 1)

    return False


def delArray(o_obj, i_edit):

    if not isinstance(o_obj, dict):
        return

    # Iterate object to search delarray
    for name in o_obj:

        if isinstance(o_obj[name], dict):
            for obj in o_obj[name]:
                # Recursively going deeper to find delarray:
                delArray(o_obj[name], i_edit)
            continue

        arr = o_obj[name]
        if not isinstance(arr, list):
            continue

        if len(arr) == 0:
            continue

        if i_edit['delarray'] == name:
            # Iteate delarray
            i = 0
            while i < len(arr):
                # Array member to delete should be dict
                if not isinstance(arr[i], dict):
                    i += 1
                    continue

                deleted = False
                # Iterate objects to delete:
                for delobj in i_edit['objects']:
                    allkeysequal = True

                    # Iterate all provided keys to check that all are equal
                    for key in delobj:
                        if key in arr[i]:
                            if arr[i][key] == delobj[key]:
                                continue

                        allkeysequal = False
                        break

                    if allkeysequal:
                        del arr[i]
                        deleted = True
                        break

                if not deleted:
                    i += 1

        # Recursively going deeper to find delarray:
        for obj in arr:
            delArray(obj, i_edit)


def replaceObject(o_obj, i_obj):
    if not isinstance(o_obj, dict):
        return

    for name in o_obj:
        if isinstance(o_obj[name], dict):
            replaceObject(o_obj[name], i_obj)
            continue
        if isinstance(o_obj[name], list):
            for obj in o_obj[name]:
                if isinstance(obj, dict):
                    replaceObject(obj, i_obj)

    if 'id' in o_obj and o_obj['id'] == i_obj['id']:
        for key in i_obj:
            if key != 'id':
                o_obj[key] = i_obj[key]


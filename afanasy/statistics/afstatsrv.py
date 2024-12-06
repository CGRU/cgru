import cgi
import json
import os
import sys
import traceback

# Python <-> PostgreSQL module
import psycopg2

os.umask(0000)

APP_FILE = os.path.realpath(__file__)
APP_ROOT = os.path.dirname(APP_FILE)

CGRU_LOCATION = os.path.dirname(os.path.dirname(APP_ROOT))
with open(os.path.join(CGRU_LOCATION,'version.txt')) as f:
    CGRU_VERSION = f.read().strip()

os.environ['CGRU_LOCATION'] = CGRU_LOCATION
os.environ['CGRU_VERSION'] = CGRU_VERSION
os.environ['AF_ROOT'] = os.path.join(CGRU_LOCATION, 'afanasy')

sys.path.append(APP_ROOT)
sys.path.append(os.path.join(CGRU_LOCATION, 'lib', 'python'))
sys.path.append(os.path.join(CGRU_LOCATION, 'afanasy', 'python'))

import cgruconfig

class Requests:
    def __init__(self, i_environ = None):
        self.environ = dict()
        self.environ['CGRU_LOCATION'] = CGRU_LOCATION
        self.environ['CGRU_VERSION'] = CGRU_VERSION
        self.environ['python'] = sys.version

        # Store some server environment, just inform client
        if i_environ:
            self.environ['environ'] = dict()
            envvars = ['SERVER_SOFTWARE','mod_wsgi.version','REMOTE_ADDR']
            for var in envvars:
                if var in i_environ:
                    self.environ['environ'][var] = i_environ[var]

        # Open DB connection:
        self.dbconn = psycopg2.connect(cgruconfig.VARS['af_db_conninfo'])
        self.cursor = self.dbconn.cursor()


    def execQuery(self, i_query):
        self.cursor.execute(i_query)
        result = []
        while (line := self.cursor.fetchone()) is not None:
            record = dict()
            col = 0
            for desc in self.cursor.description:
                record[desc[0]] = line[col]
                col += 1
            result.append(record)
        return result


    def req_init(self, i_args, o_out):
        # Return server environmet to inform client
        server = dict()
        for var in self.environ:
            server[var] = self.environ[var]
        o_out['server'] = server

        query = "SELECT min(folder) as folder FROM jobs GROUP BY folder ORDER BY folder;"
        result = self.execQuery(query)
        o_out['folders'] = []
        for line in result:
            if 'folder' in line:
                o_out['folders'].append(line['folder'])


    def req_get_jobs_folders(self, i_args, o_out):
        table = 'jobs'

        select   = i_args['select']
        time_min = i_args['time_min']
        time_max = i_args['time_max']
        folder   = i_args['folder'].rstrip('/')
        f_depth  = folder.count('/') + 1
        order    = 'jobs_quantity'

        o_out['select'] = select;
        o_out['table']  = [];

        # Select:
        query=("SELECT min(folder) AS folder," \
        "\n sum(1) AS jobs_quantity," \
        "\n sum(tasks_quantity) AS tasks_quantity," \
        "\n sum(run_time_sum)::float AS run_time_sum," \
        "\n avg(CASE WHEN tasks_done>0 THEN run_time_sum/tasks_done ELSE 0 END)::float AS run_time_avg" \
        "\n FROM " + table + "" \
        "\n WHERE time_done BETWEEN " + str(time_min) + " AND " + str(time_max) + " AND folder LIKE '" + folder + "%'" \
        "\n GROUP BY (regexp_split_to_array(btrim(folder,'/'),'/'))[" + str(f_depth) + "]" \
        "\n ORDER BY " + order + " DESC;")
 
        result = self.execQuery(query)

        for line in result:
            where = line[select]
            names = where.split('/')
            names = names[:f_depth + 1]
            where = '/'.join(names)

            # Get folder favorite service:
            sub_query=("SELECT service," \
            "\n sum(1) AS quantity"\
            "\n FROM jobs"\
            "\n WHERE folder LIKE '" + where + "%'"\
            "\n AND time_done BETWEEN " + str(time_min) + " AND " + str(time_max) + ""\
            "\n GROUP BY service ORDER BY quantity DESC;")

            sub_result = self.execQuery(sub_query)
            sub_total = 0
            sub_favourite = 0
            sub_name = 0
            for sub_line in sub_result:
                if sub_favourite < sub_line['quantity']:
                    sub_favourite = sub_line['quantity']
                    sub_name = sub_line['service']
                sub_total += sub_line['quantity']
            line['fav_service'] = sub_name
            if sub_total > 0:
                line['fav_service_percent'] = sub_favourite/sub_total

		    # Get folder favorite user:
            sub_query=("SELECT username,"\
            "\n sum(1) AS quantity"\
            "\n FROM jobs"\
            "\n WHERE folder LIKE '" + where + "%'"\
            "\n AND time_done BETWEEN " + str(time_min) + " and " + str(time_max) + ""\
            "\n GROUP BY username ORDER BY quantity DESC;")

            sub_result = self.execQuery(sub_query)
            sub_total = 0
            sub_favourite = 0
            sub_name = 0

            for sub_line in sub_result:
                if sub_favourite < sub_line['quantity']:
                    sub_favourite = sub_line['quantity']
                    sub_name = sub_line['username']
                sub_total += sub_line['quantity']
            line['fav_user'] = sub_name
            if sub_total > 0:
                line['fav_user_percent'] = sub_favourite/sub_total

            o_out['table'].append(line)


    def req_get_jobs_table(self, i_args, o_out):
        order_s = 'run_time_sum'
        order_u = 'run_time_sum'
        table = 'jobs'

        select   = i_args['select']
        favorite = i_args['favorite']
        time_min = i_args['time_min']
        time_max = i_args['time_max']
        folder   = i_args['folder'].rstrip('/')

        if 'order_u' in i_args: order_u = i_args['order_u']
        if 'order_s' in i_args: order_s = i_args['order_s']

        o_out['select']   = select
        o_out['favorite'] = favorite
        o_out['table']    = []

        # Select:
        query=("SELECT " + select + ","\
        "\n sum(1) AS jobs_quantity,"\
        "\n sum(tasks_quantity) AS tasks_quantity,"\
        "\n sum(tasks_quantity)/sum(1) AS tasks_quantity_avg,"\
        "\n avg(capacity)::float AS capacity_avg,"\
        "\n sum(run_time_sum)::float AS run_time_sum,"\
        "\n avg(CASE WHEN tasks_done>0 THEN run_time_sum/tasks_done ELSE 0 END)::float AS run_time_avg,"\
        "\n avg(tasks_done/tasks_quantity)::float AS tasks_done_percent"\
        "\n FROM " + table + " WHERE"\
        "\n time_done BETWEEN " + str(time_min) + " and " + str(time_max) + " AND"\
        "\n folder LIKE '" + folder + "%'"\
        "\n GROUP BY " + select + " ORDER BY " + order_s + " DESC;")

        result = self.execQuery(query)
        for line in result:
            # Get service favorite user:
            sub_query=("SELECT " + favorite + ","\
            "\n sum(1) AS quantity"\
            "\n FROM " + table + " WHERE"\
            "\n " + select + "='" + line[select] + "' AND"\
            "\n time_done BETWEEN " + str(time_min) + " and " + str(time_max) + " AND"\
            "\n folder LIKE '" + folder + "%'"\
            "\n GROUP BY " + favorite + " ORDER BY quantity DESC;")

            sub_result = self.execQuery(sub_query)
            sub_total = 0
            sub_favourite = 0
            sub_name = 0

            for sub_line in sub_result:
                if sub_favourite < sub_line['quantity']:
                    sub_favourite = sub_line['quantity']
                    sub_name = sub_line[favorite]
                sub_total += sub_line['quantity']

            line['fav_name'] = sub_name
            if sub_total > 0:
                line['fav_percent'] = sub_favourite/sub_total

            o_out['table'].append(line)


    def req_get_tasks_folders(self, i_args, o_out):
        table = 'tasks'

        select   = i_args['select']
        time_min = i_args['time_min']
        time_max = i_args['time_max']
        folder   = i_args['folder'].rstrip('/')
        f_depth  = folder.count('/') + 1
        order    = 'tasks_quantity'

        o_out['select'] = select
        o_out['table']  = []

        query=("SELECT min(" + select + ") as " + select + ","\
        "\n sum(1) AS tasks_quantity,"\
        "\n avg(capacity)::float AS capacity_avg,"\
        "\n sum(time_done-time_started)::float AS run_time_sum,"\
        "\n avg(time_done-time_started)::float AS run_time_avg,"\
        "\n avg(error)::float AS error_avg"\
        "\n FROM " + table + ""\
        "\n WHERE time_done BETWEEN " + str(time_min) + " and " + str(time_max) + " AND folder LIKE '" + folder + "%'"\
        "\n GROUP BY (regexp_split_to_array(btrim(folder,'/'),'/'))[" + str(f_depth) + "]"\
        "\n ORDER BY " + order + " DESC;")
         
        result = self.execQuery(query)
        for line in result:
            where = line[select];
            names = where.split('/')
            names = names[:f_depth + 1]
            where = '/'.join(names)

            # Get folder favorite service:
            sub_query=("SELECT service,"\
            "\n sum(1) AS quantity"\
            "\n FROM tasks"\
            "\n WHERE folder LIKE '" + where + "%'"\
            "\n AND time_done BETWEEN " + str(time_min) + " and " + str(time_max) + ""\
            "\n GROUP BY service ORDER BY quantity DESC;")

            sub_result = self.execQuery(sub_query)
            sub_total = 0
            sub_favourite = 0
            sub_name = 0
            for sub_line in sub_result:
                if sub_favourite < sub_line['quantity']:
                    sub_favourite = sub_line['quantity']
                    sub_name = sub_line['service']
                sub_total += sub_line['quantity']

            line['fav_service'] = sub_name
            if sub_total > 0:
                line['fav_service_percent'] = sub_favourite/sub_total;

            # Get folder favorite user:
            sub_query=("SELECT username,"\
            "\n sum(1) AS quantity"\
            "\n FROM tasks"\
            "\n WHERE folder LIKE '" + where + "%'"\
            "\n AND time_done BETWEEN " + str(time_min) + " and " + str(time_max) + ""\
            "\n GROUP BY username ORDER BY quantity DESC;")

            sub_result = self.execQuery(sub_query)
            sub_total = 0
            sub_favourite = 0
            sub_name = 0
            for sub_line in sub_result:
                if sub_favourite < sub_line['quantity']:
                    sub_favourite = sub_line['quantity']
                    sub_name = sub_line['username']
                sub_total += sub_line['quantity']

            line['fav_user'] = sub_name;
            if sub_total > 0:
                line['fav_user_percent'] = sub_favourite/sub_total

            o_out['table'].append(line)


    def req_get_tasks_table(self, i_args, o_out):
        table = 'tasks'
        select   = i_args['select']
        favorite = i_args['favorite']
        time_min = i_args['time_min']
        time_max = i_args['time_max']
        folder   = i_args['folder']

        order_s = 'run_time_sum'
        order_u = 'run_time_sum'

        if 'order_u' in i_args: order_u = i_args['order_u']
        if 'order_s' in i_args: order_s = i_args['order_s']

        o_out['select']   = select
        o_out['favorite'] = favorite
        o_out['table']    = []

        query=("SELECT " + select + ","\
        "\n sum(1) AS tasks_quantity,"\
        "\n avg(capacity)::float AS capacity_avg,"\
        "\n sum(time_done-time_started)::float AS run_time_sum,"\
        "\n avg(time_done-time_started)::float AS run_time_avg,"\
        "\n avg(error)::float AS error_avg"\
        "\n FROM " + table + ""\
        "\n WHERE time_done BETWEEN " + str(time_min) + " and " + str(time_max) + " AND folder LIKE '" + folder + "%'"\
        "\n GROUP BY " + select + " ORDER BY " + order_s + " DESC;")

        result = self.execQuery(query)

        for line in result:
            # Get service favorite user:
            sub_query=("SELECT " + favorite + ","\
            "\n sum(1) AS tasks_quantity"\
            "\n FROM " + table + ""\
            "\n WHERE " + select + "='" + line[select] + "'"\
            "\n AND time_done BETWEEN " + str(time_min) + " and " + str(time_max) + " AND folder LIKE '" + folder + "%'"\
            "\n GROUP BY " + favorite + " ORDER BY tasks_quantity DESC;")

            sub_result = self.execQuery(sub_query)
            sub_total = 0
            sub_favourite = 0
            sub_name = 0
            for sub_line in sub_result:
                if sub_favourite < sub_line["tasks_quantity"]:
                    sub_favourite = sub_line["tasks_quantity"]
                    sub_name = sub_line[favorite]
                sub_total += sub_line["tasks_quantity"]

            line['fav_name'] = sub_name;
            if sub_total > 0:
                line['fav_percent'] = sub_favourite/sub_total

            o_out['table'].append(line)


    def req_get_tasks_folders_graph(self, i_args, o_out):
        table = 'tasks'

        select   = i_args['select']
        time_min = i_args['time_min']
        time_max = i_args['time_max']
        interval = i_args['interval']
        folder   = i_args['folder'].rstrip('/')
        f_depth  = folder.count('/') + 1
        order    = 'quantity'

        o_out['time_min'] = time_min
        o_out['time_max'] = time_max
        o_out['interval'] = interval
        o_out['select'] = select
        o_out['table'] = []
        o_out['graph'] = dict()

        # Query whole time interval table:
        query=("SELECT min(folder) as folder,"\
        "\n sum(1) AS quantity"\
        "\n FROM " + table + ""\
        "\n WHERE time_done BETWEEN " + str(time_min) + " and " + str(time_max) + " AND folder LIKE '" + folder + "%'"\
        "\n GROUP BY (regexp_split_to_array(btrim(folder,'/'),'/'))[" + str(f_depth) + "]"\
        "\n ORDER BY quantity DESC;")

        result = self.execQuery(query)
        for line in result:
            o_out['table'].append(line)

        # Query graph (a table per time interval):
        time = time_min
        while time <= time_max:
            cur_time_min = time
            cur_time_max = time + interval
            o_out['graph'][time] = dict()

            query=("SELECT min(folder) as folder,"\
            "\n sum(1) AS quantity"\
            "\n FROM " + table + ""\
            "\n WHERE time_done BETWEEN " + str(cur_time_min) + " and " + str(cur_time_max) + " AND folder LIKE '" + folder + "%'"\
            "\n GROUP BY (regexp_split_to_array(btrim(folder,'/'),'/'))[" + str(f_depth) + "]"\
            "\n ORDER BY quantity DESC;")

            result = self.execQuery(query)
            for line in result:
                fname = line[select]
                fname = fname.split('/')
                fname = fname[f_depth : f_depth + 1]
                if len(fname):
                    fname = fname[0]
                else:
                    fname = ''

                o_out['graph'][time][fname] = line

            time += interval


    def req_get_tasks_graph(self, i_args, o_out):

        table    = 'tasks'
        time_min = i_args['time_min']
        time_max = i_args['time_max']
        interval = i_args['interval']
        folder   = i_args['folder']
        select   = i_args['select']

        o_out['time_min'] = time_min
        o_out['time_max'] = time_max
        o_out['interval'] = interval
        o_out['select'] = select
        o_out['table'] = []
        o_out['graph'] = dict()

        # Query whole time interval table:
        query=("SELECT " + select + ","\
        "\n sum(1) AS quantity"\
        "\n FROM " + table + ""\
        "\n WHERE time_done BETWEEN " + str(time_min) + " and " + str(time_max) + " AND folder LIKE '" + folder + "%'"\
        "\n GROUP BY " + select + " ORDER BY quantity DESC;")

        result = self.execQuery(query)
        for line in result:
            o_out['table'].append(line)

        # Query graph (a table per time interval):
        time = time_min;
        while time <= time_max:
            cur_time_min = time
            cur_time_max = time + interval
            o_out['graph'][time] = dict()

            query=("SELECT " + select + ","\
            "\n sum(1) AS quantity"\
            "\n FROM " + table + ""\
            "\n WHERE time_done BETWEEN " + str(cur_time_min) + " and " + str(cur_time_max) + " AND folder LIKE '" + folder + "%'"\
            "\n GROUP BY " + select + " ORDER BY quantity DESC;")

            result = self.execQuery(query)
            for line in result:
                o_out['graph'][time][line[select]] = line

            time += interval;


    def req_folder_delete(self, i_args, o_out):
        folder = i_args['folder']

        cursor = self.dbconn.cursor()

        query = "DELETE FROM jobs WHERE folder LIKE '" + folder + "';"
        cursor.execute(query)
        o_out['deleted_jobs'] = cursor.rowcount

        query = "DELETE FROM tasks WHERE folder LIKE '" + folder + "';"
        cursor.execute(query)
        o_out['deleted_tasks'] = cursor.rowcount


def application(environ, start_response):

    out = dict()

    requests = Requests(environ)

    request = None

    content_length = 0
    if 'CONTENT_LENGTH' in environ:
        content_length = int(environ['CONTENT_LENGTH'])

    if content_length:
        try:
            request = json.load(environ['wsgi.input'])
        except:
            request = None
            out['error'] = 'Request json load error.'
            out['info'] = '%s' % traceback.format_exc()

    rawout = None
    if request and len(request):
        for key in request:
            func = 'req_%s' % key
            if hasattr(requests, func):
                rawout = getattr(requests, func)(request[key], out)
            else:
                out['error'] = 'Request not recognized: ' + key
                break
    else:
        out['error'] = 'Request is empty.'

    status = '200 OK'
    if rawout is None:
        rawout = json.dumps(out)
    rawout = rawout.encode()
    response_headers = [('Content-type', 'text/plain'), ('Content-Length', str(len(rawout)))]
    start_response(status, response_headers)

    return [rawout]


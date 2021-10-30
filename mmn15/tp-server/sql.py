import sqlite3
from objects import User, Message


class SQLHandler(object):
    SELECT_USERS_TABLE = "SELECT name FROM sqlite_master WHERE type='table' AND name='users';"
    SELECT_MESSAGES_TABLE = "SELECT name FROM sqlite_master WHERE type='table' AND name='messages';"
    CREATE_USERS_TABLE = "CREATE TABLE users(ID BLOB PRIMARY KEY, Name BLOB, PubKey BLOB, LastSeen TEXT)"
    CREATE_MESSAGE_TABLE = "CREATE TABLE messages(ID BLOB PRIMARY KEY, ToCLient BLOB, FromClient BLOB, Type BLOB, Content BLOB)"

    SELECT_ALL_USERS_FROM_DB = "SELECT * FROM users;"
    SELECT_USER_FROM_DB = "SELECT * FROM users WHERE ID=(?);"
    INSERT_USER = "INSERT INTO users (ID, Name, PubKey, LastSeen) VALUES (?, ?, ?, ?)"
    UPDATE_USER_LAST_SEEN = 'UPDATE users SET LastSeen = (?) where id = (?)'

    SELECT_ALL_MESSAGES_FROM_DB = "SELECT * FROM messages;"
    INSERT_MESSAGE = "INSERT INTO message (ID, ToCLient, FromClient, Type, Content) VALUES (?, ?, ?, ?, ?)"
    DELETE_MESSAGE_FROM_DB = "DELETE FROM message WHERE ToCLient=(?);"


    def __init__(self, db_path):
        self.db_path = db_path
        self._init_db()

    def _init_db(self):
        con = self._open_db()
        try:
            cursorObj = con.cursor()
            cursorObj.execute(self.SELECT_USERS_TABLE)
            if len(cursorObj.fetchall()) == 0:
                cursorObj.execute(self.CREATE_USERS_TABLE)
                con.commit()
            cursorObj.execute(self.SELECT_MESSAGES_TABLE)
            if len(cursorObj.fetchall()) == 0:
                cursorObj.execute(self.CREATE_MESSAGE_TABLE)
                con.commit()
        except Exception as e:
            print(f'Couldn\'t initialize db')
        finally:
            con.close()

    def _open_db(self):
        try:
            con = sqlite3.connect(self.db_path)
            return con
        except Exception as e:
            print(f'Couldn\'t connect to db: {self.db_path}')
            raise e

    def load_users_from_db(self):
        con = self._open_db()
        try:
            users_dict = {}
            cursorObj.execute(self.SELECT_ALL_USERS_FROM_DB)
            users = cursorObj.fetchall()
            for user in users:
                user_obj = User(*user)
                users_dict[user_obj.user_id] = user_obj
            return users_dict
        except Exception as e:
            print(f'Couldn\'t load users from db')
        finally:
            con.close()
        
    def load_messages_from_db(self):
        con = self._open_db()
        try:
            messages_dict = {}
            cursorObj.execute(self.SELECT_ALL_MESSAGES_FROM_DB)
            messages = cursorObj.fetchall()
            for message in messages:
                message_obj = Message(*message)
                if message_obj.to_client not in messages_dict:
                    messages_dict[message_obj.to_client] = []
                messages_dict[message_obj.to_client].append(message_obj)
            return messages_dict
        except Exception as e:
            print(f'Couldn\'t load messages from db')
        finally:
            con.close()

    def update_user_to_db(self, user):
        con = self._open_db()
        try:
            cursorObj = con.cursor()
            cursorObj.execute(self.SELECT_USER_FROM_DB, (user.user_id,))
            if len(cursorObj.fetchall()) == 0:
                user_data = (user.user_id, user.name, user.public_key, user.last_seen)
                cursorObj.execute(self.INSERT_USER, user_data)
                con.commit()
            else:
                user_data = (user.last_seen, user.user_id)
                cursorObj.execute(self.UPDATE_USER_LAST_SEEN, user_data)
                con.commit()
        except Exception as e:
            print(f'Couldn\'t initialize user in db')
        finally:
            con.close()

    def add_message_to_db(self, message):
        con = self._open_db()
        try:
            cursorObj = con.cursor()
            message_data = (message.message_id, message.to_client, message.from_client, message.message_type, message.content)
            cursorObj.execute(self.INSERT_MESSAGE, message_data)
            con.commit()
        except Exception as e:
            print(f'Couldn\'t initialize message in db')
        finally:
            con.close()

    def remove_client_messages(self, client_id):
        con = self._open_db()
        try:
            cursorObj = con.cursor()
            cursorObj.execute(self.DELETE_MESSAGE_FROM_DB, (client_id,))
            con.commit()
        except Exception as e:
            print(f'Couldn\'t delete messages from db')
        finally:
            con.close()

#include <stdlib.h>
#include <string.h>
#include <lwrapper.h>
#include <gtest/gtest.h>

TEST(TestLWrapper, InitDbTest) {
  const db_store_t* db = init_db_store("test_db", true);
  ASSERT_TRUE(db);

  destroy_db_store(db);

  db = init_db_store("test_db", false);
  ASSERT_TRUE(db);
  destroy_db_store(db);
}

TEST(TestLWrapper, InsertAndLookupTest) {
  const db_store_t* db =  init_db_store("test_db", true);
  int ret = -1;
  ASSERT_TRUE(db);
  size_t key_len = strlen("test_key");
  size_t val_len = strlen("test_value") + 1;

  db_kvpair_t  record = { .key = "test_key",
			  .val = (void*)"test_value\0",
			  .key_len = key_len,
			  .val_len = val_len,
			};

  db_kvpair_t  rev_record;
  rev_record.key = (char*)record.val;
  rev_record.key_len = record.val_len;

  //put record
  ret = put_id_handle(&record, 1, db);  
  ASSERT_FALSE(ret);

  //lookup record by id 
  record.val = NULL;
  record.val_len = 0;

  ret  = get_id_handle(&record, 1, db, false);
  ASSERT_FALSE(ret);
  ASSERT_TRUE(record.val); 
  ASSERT_FALSE(strcmp((char*)record.val, rev_record.key));
  ASSERT_TRUE(record.val_len = rev_record.key_len); 

  //lookup record by id 
  rev_record.val = NULL;
  rev_record.val_len = 0;
  ret  = get_id_handle(&rev_record, 1, db, true);
  ASSERT_FALSE(ret);
  ASSERT_TRUE(rev_record.val);
  char* str_value = (char*)malloc(sizeof(char) * (rev_record.val_len + 1));
  strcpy(str_value, record.key);
  ASSERT_FALSE(strcmp(str_value, record.key));
  ASSERT_TRUE(rev_record.val_len = record.key_len); 
 
  //delete record by id
  ret = delete_id_handle(&record, 1, db, false);
  ASSERT_FALSE(ret);

  destroy_db_store(db);
}

TEST(TestLWrapper, InsertAndDeleteTest) {
  const db_store_t* db =  init_db_store("test_db", true);
  int ret = -1;
  ASSERT_TRUE(db);
  size_t key_len = strlen("test_del_key");
  size_t val_len = strlen("test_del_value") + 1;

  db_kvpair_t  record = { .key = "test_del_key",
			  .val = (void*)"test_del_value\0",
			  .key_len = key_len,
			  .val_len = val_len,
			};

  db_kvpair_t  rev_record;
  rev_record.key = (char*)record.val;
  rev_record.key_len = record.val_len;

  //put record
  ret = put_id_handle(&record, 1, db);  
  ASSERT_FALSE(ret);

  record.val = NULL;
  record.val_len = 0;
 
  //lookup record by id 
  ret = get_id_handle(&record, 1, db, false);
  ASSERT_FALSE(ret);
  ASSERT_TRUE(record.val);
  ASSERT_FALSE(strcmp((char*)record.val, rev_record.key));
  ASSERT_TRUE(record.val_len = rev_record.key_len); 

  //delete record by id
  ret = delete_id_handle(&record, 1, db, false);
  ASSERT_FALSE(ret);
  
  //lookup record by id 
  record.val = NULL;
  record.val_len = 0;
  ret = get_id_handle(&record, 1, db, false);
  ASSERT_FALSE(ret);
  ASSERT_FALSE(record.val);
  ASSERT_FALSE(record.val_len);

  //lookup record by handle 
  rev_record.val = NULL;
  rev_record.val_len = 0;
  ret = get_id_handle(&rev_record, 1, db, true);
  ASSERT_FALSE(ret);
  ASSERT_FALSE(rev_record.val);
  ASSERT_FALSE(rev_record.val_len);
  destroy_db_store(db);
}

TEST(TestLWrapper, CommitTransactionTest) {
  const db_store_t* db = init_db_store("test_db", true);
  ASSERT_TRUE(db);

  db_kvpair_t* record = (db_kvpair_t*)malloc(sizeof(db_kvpair_t));
  char *key = "1234";
  char *value = "/a/b/c/d\0";
  size_t key_len = strlen(key);
  size_t val_len = strlen(value) + 1;
  record->key = key;
  record->val = value;
  record->key_len = key_len;
  record->val_len = val_len;

  //commit first transaction
  int ret = commit_transaction(record, 1, db);
  ASSERT_FALSE(ret);

  key = "5678";
  value = "/a/b/c/d/e\0";
  key_len = strlen(key);
  val_len = strlen(value) + 1;

  record->key = key;
  record->val = value;
  record->key_len = key_len;
  record->val_len = val_len;

  //commit second transaction
  ret = commit_transaction(record, 1, db);
  ASSERT_FALSE(ret);

  free(record);    

  // now iterate over all the transactions
  db_kvpair_t** tr_records = NULL;
  int nrecs = 0;
  ret = iterate_transactions(&tr_records, &nrecs, db);

  // verify the commited transaction
  ASSERT_FALSE(ret);
  ASSERT_TRUE(nrecs == 2);
  ASSERT_FALSE(strcmp("1234", tr_records[0]->key));
  ASSERT_FALSE(strcmp("/a/b/c/d", (char*)tr_records[0]->val));
  ASSERT_FALSE(strcmp("5678", tr_records[1]->key));
  ASSERT_FALSE(strcmp("/a/b/c/d/e", (char*)tr_records[1]->val));

  // now delete all transacations
  ret = delete_transaction(tr_records[0], 1, db);
  ASSERT_FALSE(ret);

  ret = delete_transaction(tr_records[1], 1, db);
  ASSERT_FALSE(ret);

  cleanup_transaction_iterator(tr_records, nrecs);

  // now iterate again over committed transaction
  ret = iterate_transactions(&tr_records, &nrecs, db);

  ASSERT_FALSE(ret);
  ASSERT_TRUE(nrecs == 0);
  cleanup_transaction_iterator(tr_records, nrecs);

  destroy_db_store(db);
}

TEST(TestLWrapper, CommitTransactionAndInsertTest) {
  const db_store_t* db = init_db_store("test_db", true);
  ASSERT_TRUE(db);

  db_kvpair_t* record = (db_kvpair_t*)malloc(sizeof(db_kvpair_t));
  char *key = "1234";
  char *value = "/a/b/c/d\0";
  size_t key_len = strlen(key);
  size_t val_len = strlen(value) + 1;

  record->key = key;
  record->val = value;
  record->key_len = key_len;
  record->val_len = val_len;

  //commit first transaction
  int ret = commit_transaction(record, 1, db);
  ASSERT_FALSE(ret);

  key = "5678";
  value = "/a/b/c/d/e\0";
  key_len = strlen(key);
  val_len = strlen(value) + 1;

  record->key = key;
  record->val = value;
  record->key_len = key_len;
  record->val_len = val_len;
  // insert a normal key
  ret = put_id_handle(record, 1, db);
  ASSERT_FALSE(ret);

  free(record);    

  db_kvpair_t** tr_records = NULL;
  int nrecs = -1;
  ret = iterate_transactions(&tr_records, &nrecs, db);

  ASSERT_FALSE(ret);
  // only one transaction commit should be present
  ASSERT_TRUE(nrecs == 1);
  ASSERT_FALSE(strcmp("1234", (char*)tr_records[0]->key));
  ASSERT_FALSE(strcmp("/a/b/c/d", (char*)tr_records[0]->val));

  cleanup_transaction_iterator(tr_records, nrecs);

  destroy_db_store(db);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


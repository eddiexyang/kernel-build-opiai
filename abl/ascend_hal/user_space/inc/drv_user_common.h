/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */
#ifndef _DRV_USER_COMMON_H_
#define _DRV_USER_COMMON_H_


/* user list definition */
struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

/*lint -e773 -e773*/
#define LIST_HEAD(name)  \
    struct list_head name = LIST_HEAD_INIT(name)
/*lint +e773 +e773*/

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

#define container_of(ptr, type, member) ({ \
        const typeof(((type *)0)->member)(*__mptr) = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member)); })

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)


#define list_for_each_safe(pos, n, head) \
    for ((pos) = (head)->next, (n) = (pos)->next; (pos) != (head); \
         (pos) = (n), (n) = (pos)->next)

int drv_user_list_empty(const struct list_head *head);
void drv_user_list_add_tail(struct list_head *new_node, struct list_head *head);
void drv_user_list_add_head(struct list_head *new_node, struct list_head *head);
void drv_user_list_del(struct list_head *entry);

int drvGetRuntimeApiVer(void);

#endif

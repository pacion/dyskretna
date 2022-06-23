#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "defs.h"

int main() {

    struct student_t **students;

    int res = read_students_grades("test.txt", &students);

    if (res == -2)
    {
        printf("Couldn't open file");
        return 4;
    }
    else if (res == -3)
    {
        printf("File corrupted");
        return 6;
    }
    else if (res == -4)
    {
        printf("Failed to allocate memory");
        return 8;
    }

    display_students_grades(students);

    sort_students_grades(&students);

    display_students_grades(students);

    free_students_grades(students);

    return 0;
}

int read_students_grades(const char *filename, struct student_t ***students)
{
    if (filename == NULL || students == NULL)
        return -1;

    FILE *f = fopen(filename, "rt");

    if (f == NULL)
        return -2;

    struct student_t **local_students = NULL;
    int number_of_students = 0;

    while (!feof(f))
    {
        char name[21], last_name[41], course[43];
        int grade;

        int res = fscanf(f, "%20[^|]|%40[^|]|%42[^|]|%d\n", name, last_name, course, &grade);

        if (res == EOF)
            break;
        else if (res != 4)
        {
            free_students_grades(local_students);
            fclose(f);
            return -3;
        }

        struct student_t *student = NULL;

        for (int i = 0; i < number_of_students; ++i)
        {
            if (strcmp((*(local_students + i))->name, name) == 0 && strcmp((*(local_students + i))->last_name, last_name) == 0)
            {
                student = *(local_students + i);
                break;
            }
        }

        if (student)
        {
            size_t course_id = 0;
            for (; course_id < student->number_of_courses; ++course_id)
            {
                if (strcmp((student->courses + course_id)->course, course) == 0)
                    break;
            }

            if (course_id == student->number_of_courses)
            {
                struct course_grades_t *temp = realloc(student->courses, sizeof(struct course_grades_t) * (student->number_of_courses + 1));

                if (temp == NULL)
                {
                    free_students_grades(local_students);
                    fclose(f);
                    return -4;
                }

                student->courses = temp;
                strcpy((student->courses + student->number_of_courses)->course, course);

                (student->courses + student->number_of_courses)->grades = calloc(1, sizeof(int));

                student->number_of_courses++;

                if ((student->courses + student->number_of_courses - 1)->grades == NULL)
                {
                    free_students_grades(local_students);
                    fclose(f);
                    return -4;
                }

                *(student->courses + student->number_of_courses - 1)->grades = grade;
                (student->courses + student->number_of_courses - 1)->number_of_grades = 1;
            }
            else
            {
                int *grades = realloc((student->courses + course_id)->grades, sizeof(int) * ((student->courses + course_id)->number_of_grades + 1));

                if (grades == NULL)
                {
                    free_students_grades(local_students);
                    fclose(f);
                    return -4;
                }

                (student->courses + course_id)->grades = grades;
                *((student->courses + course_id)->grades + (student->courses + course_id)->number_of_grades++) = grade;
            }

        }
        else
        {
            struct student_t **temp = realloc(local_students, sizeof(struct student_t *) * (number_of_students + 2));

            if (temp == NULL)
            {
                free_students_grades(local_students);
                fclose(f);
                return -4;
            }

            local_students = temp;

            *(local_students + number_of_students + 1) = NULL;
            *(local_students + number_of_students) = calloc(1, sizeof(struct student_t));

            if (*(local_students + number_of_students) == NULL)
            {
                free_students_grades(local_students);
                fclose(f);
                return -4;
            }

            strcpy((*(local_students + number_of_students))->name, name);
            strcpy((*(local_students + number_of_students))->last_name, last_name);

            (*(local_students + number_of_students))->courses = calloc(1, sizeof(struct course_grades_t));

            if ((*(local_students + number_of_students))->courses == NULL)
            {
                free_students_grades(local_students);
                fclose(f);
                return -4;
            }

            (*(local_students + number_of_students))->number_of_courses = 1;

            strcpy( (*(local_students + number_of_students))->courses->course, course);

            (*(local_students + number_of_students))->courses->grades = calloc(1, sizeof(int));

            if ((*(local_students + number_of_students))->courses->grades == NULL)
            {
                free_students_grades(local_students);
                fclose(f);
                return -4;
            }

            (*(local_students + number_of_students))->courses->number_of_grades = 1;
            *(*(local_students + number_of_students))->courses->grades = grade;
            number_of_students++;
        }
    }


    *students = local_students;

    fclose(f);

    return 0;
}

void display_students_grades(struct student_t **students)
{
    if (students == NULL)
        return;

    for (int i = 0; *(students + i); ++i)
    {
        printf("%s %s\n", (*(students + i))->name, (*(students + i))->last_name);

        for (size_t j = 0; j < (*(students + i))->number_of_courses; ++j)
        {
            printf("-%s: ", ((*(students + i))->courses + j)->course);

            for (size_t k = 0; k < ((*(students + i))->courses + j)->number_of_grades; ++k)
            {
                printf("%d ", *(((*(students + i))->courses + j)->grades + k));
            }
            printf("\n");
        }
    }
}

void free_students_grades(struct student_t **students)
{
    if (students == NULL)
        return;

    for (int i = 0; *(students + i); ++i)
    {
        for (size_t j = 0; j < (*(students + i))->number_of_courses; ++j)
            free(((*(students + i))->courses + j)->grades);

        free((*(students + i))->courses);
        free((*(students + i)));
    }

    free(students);

}

int write_student_grades(const char *filename, struct student_t** students, const char* name, const char* last_name)
{
    if (filename == NULL || students == NULL || name == NULL || last_name == NULL)
        return -1;

    FILE *f = NULL;

    for (int i = 0; *(students + i); ++i)
    {
        if (strcmp((*(students + i))->name, name) == 0 && strcmp((*(students + i))->last_name, last_name) == 0)
        {
            for (size_t j = 0; j < (*(students + i))->number_of_courses; ++j)
            {
                for (size_t k = 0; k < ((*(students + i))->courses + j)->number_of_grades; ++k)
                {
                    unsigned char name_length = strlen(name);
                    unsigned char last_name_length = strlen(last_name);
                    unsigned char course_length = strlen(((*(students + i))->courses + j)->course);
                    unsigned char grade = *(((*(students + i))->courses + j)->grades + k);

                    if (f == NULL)
                    {
                        f = fopen(filename, "wb");
                        if (f == NULL)
                            return -2;
                    }

                    fwrite(&name_length, sizeof(char ), 1, f);
                    fwrite(name, sizeof(char), name_length, f);
                    fwrite(&last_name_length, sizeof(char ), 1, f);
                    fwrite(last_name, sizeof(char), last_name_length, f);
                    fwrite(&course_length, sizeof(char ), 1, f);
                    fwrite(((*(students + i))->courses + j)->course, sizeof(char), course_length, f);
                    fwrite(&grade, sizeof(char ), 1, f);

                }
            }
            break;
        }
    }

    if (f == NULL)
        return -3;

    fclose(f);
    return 0;
}

int write_course_grades(const char *filename, struct student_t** students, const char* course)
{
    if (filename == NULL || students == NULL || course == NULL)
        return -1;

    FILE *f = NULL;

    for (int i = 0; *(students + i); ++i)
    {
        for (size_t j = 0; j < (*(students + i))->number_of_courses; ++j)
        {
            if (strcmp(((*(students + i))->courses + j)->course, course) == 0) {
                for (size_t k = 0; k < ((*(students + i))->courses + j)->number_of_grades; ++k) {
                    unsigned char name_length = strlen((*(students + i))->name);
                    unsigned char last_name_length = strlen((*(students + i))->last_name);
                    unsigned char course_length = strlen(((*(students + i))->courses + j)->course);
                    unsigned char grade = *(((*(students + i))->courses + j)->grades + k);

                    if (f == NULL) {
                        f = fopen(filename, "wb");
                        if (f == NULL)
                            return -2;
                    }

                    fwrite(&name_length, sizeof(char), 1, f);
                    fwrite((*(students + i))->name, sizeof(char), name_length, f);
                    fwrite(&last_name_length, sizeof(char), 1, f);
                    fwrite((*(students + i))->last_name, sizeof(char), last_name_length, f);
                    fwrite(&course_length, sizeof(char), 1, f);
                    fwrite(((*(students + i))->courses + j)->course, sizeof(char), course_length, f);
                    fwrite(&grade, sizeof(char), 1, f);
                }
            }
        }
    }

    if (f == NULL)
        return -3;

    fclose(f);
    return 0;
}

int sort_students_grades(struct student_t*** students)
{
    if (students == NULL)
        return -1;

    for (int i = 0; *(*students + i); ++i)
        for (int j = i + 1; *(*students + j); ++j)
        {
            if (strcmp((*(*students + i))->last_name, (*(*students + j))->last_name) > 0)
            {
                struct student_t *student = *(*students + i);
                *(*students + i) = *(*students + j);
                *(*students + j) = student;
            }
        }

    return 0;
}

int add_grade(struct student_t*** students, const char* name, const char* last_name, const char* course, int grade)
{
    if (students == NULL || course == NULL || name == NULL || last_name == NULL)
        return -1;

    for (int i = 0; *(*students + i); ++i)
    {
        if (strcmp((*(*students + i))->name, name) == 0 && strcmp((*(*students + i))->last_name, last_name) == 0)
        {
            for (size_t j = 0; j < (*(*students + i))->number_of_courses; ++j)
            {
               if (strcmp(((*(*students + i))->courses + j)->course, course) == 0)
               {
                   int *temp = realloc(((*(*students + i))->courses + j)->grades, sizeof(int) * (((*(*students + i))->courses + j)->number_of_grades + 1));

                   if (temp == NULL)
                       return -4;

                   ((*(*students + i))->courses + j)->grades = temp;
                   *(((*(*students + i))->courses + j)->grades + ((*(*students + i))->courses + j)->number_of_grades++) = grade;
                   return 0;
               }
            }
        }
    }

    return 0;
}
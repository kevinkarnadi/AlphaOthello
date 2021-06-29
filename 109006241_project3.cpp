#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>

struct Point
{
    int x, y;
    Point() : Point(0, 0) {}
    Point(float x, float y) : x(x), y(y) {}
    bool operator==(const Point& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }
    bool operator!=(const Point& rhs) const
    {
        return !operator==(rhs);
    }
    Point operator+(const Point& rhs) const
    {
        return Point(x + rhs.x, y + rhs.y);
    }
    Point operator-(const Point& rhs) const
    {
        return Point(x - rhs.x, y - rhs.y);
    }
};

const int SIZE = 8;
std::vector<Point> next_valid_spots;
const int MAXDEPTH = 5;

class OthelloBoard
{
public:
    enum SPOT_STATE
    {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{ {
            Point(-1, -1), Point(-1, 0), Point(-1, 1),
            Point(0, -1), /*{0, 0}, */Point(0, 1),
            Point(1, -1), Point(1, 0), Point(1, 1)
        } };
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
private:
    int get_next_player(int player) const
    {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const
    {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const
    {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc)
    {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const
    {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const
    {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir : directions)
        {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY)
            {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center)
    {
        for (Point dir : directions)
        {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({ p });
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY)
            {
                if (is_disc_at(p, cur_player))
                {
                    for (Point s : discs)
                    {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard()
    {
        reset();
    }
    void reset()
    {
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8 * 8 - 4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
    }
    std::vector<Point> get_valid_spots() const
    {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    void put_disc(Point p)
    {
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);

        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();

        if (next_valid_spots.size() == 0)
        {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
        }
    }
};

OthelloBoard now_board;

void read_board(std::ifstream& fin)
{
    fin >> now_board.cur_player;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            fin >> now_board.board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin)
{
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++)
    {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

const int stateValue(const OthelloBoard b)
{
    int val = b.disc_count[1] - b.disc_count[2];
    return val;
}

int index;

int MiniMax(const OthelloBoard now, int depth, int A, int B, int player)
{
    int n_valid_spots = now.next_valid_spots.size();

    if (depth == 0 || n_valid_spots <= 0)
        return stateValue(now);

    if (player == 1)  // player's turn (maximize value)
    {
        int value = -1000;
        for (int i = 0; i < n_valid_spots; i++)
        {
            OthelloBoard next = now;
            next.put_disc(now.next_valid_spots[i]);
            int temp = MiniMax(next, depth - 1, A, B, 3 - player);
            if (temp > value)
            {
                value = temp;
                if(depth == MAXDEPTH)
                    index = i;
            }
            if (value > A)
            {
                A = value;
            }
            if (A >= B)
                break;
        }
        return value;
    }
    else if (player == 2)  // enemy's turn (minimize value)
    {
        int value = 1000;
        for (int i = 0; i < n_valid_spots; i++)
        {
            OthelloBoard next = now;
            next.put_disc(now.next_valid_spots[i]);
            int temp = MiniMax(next, depth - 1, A, B, 3 - player);
            if (temp < value)
            {
                value = temp;
                if(depth == MAXDEPTH)
                    index = i;
            }
            if (value < B)
            {
                B = value;
            }
            if (B <= A)
                break;
        }
        return value;
    }
}

void write_valid_spot(std::ofstream& fout)
{
    int n_valid_spots = next_valid_spots.size();
    if (n_valid_spots == 0)
        return;

    now_board.next_valid_spots.clear();
    now_board.next_valid_spots = next_valid_spots;

    int bestVal = MiniMax(now_board, MAXDEPTH, -1000, 1000, 1);
    Point p = now_board.next_valid_spots[index];

    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char** argv)
{
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}

using SequoiaDB.Bson;

/** \namespace SequoiaDB
 *  \brief SequoiaDB Driver for C#.Net
 *  \author Hetiu Lin
 */
namespace SequoiaDB
{
    /** \class DBQuery
     *  \brief Database operation rules
     */
    public class DBQuery
   {
        private long skipRowsCount = 0;
        private long returnRowsCount = -1;

       /** \property Matcher
        *  \brief matching rule
        */
        public BsonDocument Matcher { get; set; }

        /** \property Selector
         *  \brief selective rule
         */
        public BsonDocument Selector { get; set; }

        /** \property OrderBy
         *  \brief ordered rule
         */
        public BsonDocument OrderBy { get; set; }

        /** \property Hint
         *  \brief sepecified access plan
         */
        public BsonDocument Hint { get; set; }

        /** \property Modifier
         *  \brief modified rule
         */
        public BsonDocument Modifier { get; set; }

        /** \property SkipRowsCount
         *  \brief documents to skip
         */
        public long SkipRowsCount
        {
            get { return skipRowsCount; }
            set { skipRowsCount = value; }
        }

        /** \property ReturnRowsCount
         *  \brief documents to return
         */
        public long ReturnRowsCount
        {
            get { return returnRowsCount; }
            set { returnRowsCount = value; }
        }
   }
}
